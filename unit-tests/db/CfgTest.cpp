/***************************************************************************/ /**
 * \file       CfgTest.cc
 * OVERVIEW:   Provides the implementation for the CfgTest class, which
 *                tests the Exp and derived classes
 ******************************************************************************/

#include "CfgTest.h"

#include "boomerang/core/BinaryFileFactory.h"
#include "boomerang/db/proc.h"
#include "boomerang/db/prog.h"
#include "boomerang/db/dataflow.h"
#include "boomerang/util/Log.h"
#include "boomerang/util/Log.h"
#include "boomerang/db/basicblock.h"

#include "boomerang-frontend/pentium/pentiumfrontend.h"

#include <QDir>
#include <QProcessEnvironment>
#include <QDebug>

#define FRONTIER_PENTIUM    (BOOMERANG_TEST_BASE "/tests/inputs/pentium/frontier")
#define SEMI_PENTIUM        (BOOMERANG_TEST_BASE "/tests/inputs/pentium/semi")
#define IFTHEN_PENTIUM      (BOOMERANG_TEST_BASE "/tests/inputs/pentium/ifthen")

static bool    logset = false;

void CfgTest::initTestCase()
{
	if (!logset) {
		logset = true;
		Boomerang::get()->setProgPath(BOOMERANG_TEST_BASE);
		Boomerang::get()->setPluginPath(BOOMERANG_TEST_BASE "/lib");
		Boomerang::get()->setLogger(new NullLogger());
	}
}


/***************************************************************************/ /**
 * \fn        CfgTest::testDominators
 * OVERVIEW:        Test the dominator frontier code
 ******************************************************************************/
#define FRONTIER_FOUR        Address(0x08048347)
#define FRONTIER_FIVE        Address(0x08048351)
#define FRONTIER_TWELVE      Address(0x080483b2)
#define FRONTIER_THIRTEEN    Address(0x080483b9)

void CfgTest::testDominators()
{
	BinaryFileFactory bff;
	IFileLoader       *pBF = bff.loadFile(FRONTIER_PENTIUM);

	QVERIFY(pBF != nullptr);

	Prog      prog(FRONTIER_PENTIUM);
	IFrontEnd *pFE = new PentiumFrontEnd(pBF, &prog, &bff);
	Type::clearNamedTypes();
	prog.setFrontEnd(pFE);
	pFE->decode(&prog);

	bool    gotMain;
	   Address addr = pFE->getMainEntryPoint(gotMain);
	QVERIFY(addr != Address::INVALID);
	Module *m = *prog.begin();
	QVERIFY(m != nullptr);
	QVERIFY(m->size() > 0);

	UserProc *pProc = (UserProc *)*(m->begin());
	Cfg      *cfg   = pProc->getCFG();
	DataFlow *df    = pProc->getDataFlow();
	df->dominators(cfg);

	// Find BB "5" (as per Appel, Figure 19.5).
	BB_IT      it;
	BasicBlock *bb = cfg->getFirstBB(it);

	while (bb && bb->getLowAddr() != FRONTIER_FIVE) {
		bb = cfg->getNextBB(it);
	}

	QVERIFY(bb);
	QString     expect_st, actual_st;
	QTextStream expected(&expect_st), actual(&actual_st);
	// expected << std::hex << FRONTIER_FIVE << " " << FRONTIER_THIRTEEN << " " << FRONTIER_TWELVE << " " <<
	//    FRONTIER_FOUR << " ";
	expected << FRONTIER_THIRTEEN << " " << FRONTIER_FOUR << " " << FRONTIER_TWELVE << " " << FRONTIER_FIVE
			 << " ";
	int n5 = df->pbbToNode(bb);
	std::set<int>::iterator ii;
	std::set<int>&          DFset = df->getDF(n5);

	for (ii = DFset.begin(); ii != DFset.end(); ii++) {
		actual << df->nodeToBB(*ii)->getLowAddr() << " ";
	}

	QCOMPARE(actual_st, expect_st);
}


/***************************************************************************/ /**
 * \fn        CfgTest::testSemiDominators
 * OVERVIEW:        Test a case where semi dominators are different to dominators
 ******************************************************************************/
#define SEMI_L    Address(0x80483b0)
#define SEMI_M    Address(0x80483e2)
#define SEMI_B    Address(0x8048345)
#define SEMI_D    Address(0x8048354)
#define SEMI_M    Address(0x80483e2)

void CfgTest::testSemiDominators()
{
	BinaryFileFactory bff;
	IFileLoader       *pBF = bff.loadFile(SEMI_PENTIUM);

	QVERIFY(pBF != 0);
	Prog      prog(SEMI_PENTIUM);
	IFrontEnd *pFE = new PentiumFrontEnd(pBF, &prog, &bff);
	Type::clearNamedTypes();
	prog.setFrontEnd(pFE);
	pFE->decode(&prog);

	bool    gotMain;
	   Address addr = pFE->getMainEntryPoint(gotMain);
	QVERIFY(addr != Address::INVALID);

	Module *m = *prog.begin();
	QVERIFY(m != nullptr);
	QVERIFY(m->size() > 0);

	UserProc *pProc = (UserProc *)(*m->begin());
	Cfg      *cfg   = pProc->getCFG();

	DataFlow *df = pProc->getDataFlow();
	df->dominators(cfg);

	// Find BB "L (6)" (as per Appel, Figure 19.8).
	BB_IT      it;
	BasicBlock *bb = cfg->getFirstBB(it);

	while (bb && bb->getLowAddr() != SEMI_L) {
		bb = cfg->getNextBB(it);
	}

	QVERIFY(bb);
	int nL = df->pbbToNode(bb);

	// The dominator for L should be B, where the semi dominator is D
	// (book says F)
	   Address actual_dom  = df->nodeToBB(df->getIdom(nL))->getLowAddr();
	   Address actual_semi = df->nodeToBB(df->getSemi(nL))->getLowAddr();
	QCOMPARE(actual_dom, SEMI_B);
	QCOMPARE(actual_semi, SEMI_D);
	// Check the final dominator frontier as well; should be M and B
	QString     expected_st, actual_st;
	QTextStream expected(&expected_st), actual(&actual_st);
	// expected << std::hex << SEMI_M << " " << SEMI_B << " ";
	expected << SEMI_B << " " << SEMI_M << " ";
	std::set<int>::iterator ii;
	std::set<int>&          DFset = df->getDF(nL);

	for (ii = DFset.begin(); ii != DFset.end(); ii++) {
		actual << df->nodeToBB(*ii)->getLowAddr() << " ";
	}

	QCOMPARE(actual_st, expected_st);
}


void CfgTest::testPlacePhi()
{
	QSKIP("Disabled.");

	BinaryFileFactory bff;
	IFileLoader       *pBF = bff.loadFile(FRONTIER_PENTIUM);
	QVERIFY(pBF != 0);

	Prog      prog(FRONTIER_PENTIUM);
	IFrontEnd *pFE = new PentiumFrontEnd(pBF, &prog, &bff);
	Type::clearNamedTypes();
	prog.setFrontEnd(pFE);
	pFE->decode(&prog);

	Module *m = *prog.begin();
	QVERIFY(m != nullptr);
	QVERIFY(m->size() > 0);

	UserProc *pProc = (UserProc *)(*m->begin());
	Cfg      *cfg   = pProc->getCFG();

	// Simplify expressions (e.g. m[ebp + -8] -> m[ebp - 8]
	cfg->sortByAddress();
	prog.finishDecode();
	DataFlow *df = pProc->getDataFlow();
	df->dominators(cfg);
	df->placePhiFunctions(pProc);

	// m[r29 - 8] (x for this program)
	SharedExp e = Unary::get(opMemOf, Binary::get(opMinus, Location::regOf(29), Const::get(4)));

	// A_phi[x] should be the set {7 8 10 15 20 21} (all the join points)
	QString     actual_st;
	QTextStream actual(&actual_st);

	std::set<int>& A_phi = df->getA_phi(e);

	for (std::set<int>::iterator ii = A_phi.begin(); ii != A_phi.end(); ++ii) {
		actual << *ii << " ";
	}

	QCOMPARE(actual_st, QString("7 8 10 15 20 21 "));
}



void CfgTest::testPlacePhi2()
{
	QSKIP("Disabled.");

	BinaryFileFactory bff;
	IFileLoader       *pBF = bff.loadFile(IFTHEN_PENTIUM);

	QVERIFY(pBF != 0);
	Prog      prog(IFTHEN_PENTIUM);
	IFrontEnd *pFE = new PentiumFrontEnd(pBF, &prog, &bff);
	Type::clearNamedTypes();
	prog.setFrontEnd(pFE);
	pFE->decode(&prog);

	Module *m = *prog.begin();
	QVERIFY(m != nullptr);
	QVERIFY(m->size() > 0);

	UserProc *pProc = (UserProc *)(*m->begin());

	// Simplify expressions (e.g. m[ebp + -8] -> m[ebp - 8]
	prog.finishDecode();

	Cfg *cfg = pProc->getCFG();
	cfg->sortByAddress();

	DataFlow *df = pProc->getDataFlow();
	df->dominators(cfg);
	df->placePhiFunctions(pProc);

	// In this program, x is allocated at [ebp-4], a at [ebp-8], and
	// b at [ebp-12]
	// We check that A_phi[ m[ebp-8] ] is 4, and that
	// A_phi A_phi[ m[ebp-8] ] is null
	// (block 4 comes out with n=4)

	/*
	 * Call (0)
	 |
	 | V
	 | Call (1)
	 |
	 | V
	 | Twoway (2) if (b < 4 )
	 |
	 |-T-> Fall (3)
	 |      |
	 |      V
	 |-F-> Call (4) ----> Ret (5)
	 */

	QString     actual_st;
	QTextStream actual(&actual_st);
	// m[r29 - 8]
	SharedExp               e = Unary::get(opMemOf, Binary::get(opMinus, Location::regOf(29), Const::get(8)));
	std::set<int>&          s = df->getA_phi(e);
	std::set<int>::iterator pp;

	for (pp = s.begin(); pp != s.end(); pp++) {
		actual << *pp << " ";
	}

	QCOMPARE(actual_st, QString("4 "));

	if (s.size() > 0) {
		BBTYPE actualType   = df->nodeToBB(*s.begin())->getType();
		BBTYPE expectedType = BBTYPE::CALL;
		QCOMPARE(actualType, expectedType);
	}

	QString     expected = "";
	QString     actual_st2;
	QTextStream actual2(&actual_st2);
	// m[r29 - 12]
	e = Unary::get(opMemOf, Binary::get(opMinus, Location::regOf(29), Const::get(12)));

	std::set<int>& s2 = df->getA_phi(e);

	for (pp = s2.begin(); pp != s2.end(); pp++) {
		actual2 << *pp << " ";
	}

	QCOMPARE(actual_st2, expected);
	delete pFE;
}


/***************************************************************************/ /**
 * \fn        CfgTest::testRenameVars
 * OVERVIEW:        Test the renaming of variables
 ******************************************************************************/
void CfgTest::testRenameVars()
{
	BinaryFileFactory bff;
	IFileLoader       *pBF = bff.loadFile(FRONTIER_PENTIUM);

	QVERIFY(pBF != 0);
	Prog      *prog = new Prog(FRONTIER_PENTIUM);
	IFrontEnd *pFE  = new PentiumFrontEnd(pBF, prog, &bff);
	Type::clearNamedTypes();
	prog->setFrontEnd(pFE);
	pFE->decode(prog);

	Module *m = *prog->begin();
	QVERIFY(m != nullptr);
	QVERIFY(m->size() > 0);

	UserProc *pProc = (UserProc *)(*m->begin());
	Cfg      *cfg   = pProc->getCFG();
	DataFlow *df    = pProc->getDataFlow();

	// Simplify expressions (e.g. m[ebp + -8] -> m[ebp - 8]
	prog->finishDecode();

	df->dominators(cfg);
	df->placePhiFunctions(pProc);
	pProc->numberStatements();        // After placing phi functions!
	df->renameBlockVars(pProc, 0, 1); // Block 0, mem depth 1

	// MIKE: something missing here?

	delete pFE;
}


QTEST_MAIN(CfgTest)
