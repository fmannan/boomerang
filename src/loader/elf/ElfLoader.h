#pragma once

/*
 * Copyright (C) 1998-2001, The University of Queensland
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

/** \file ElfBinaryFile.h
 * \brief This file contains the definition of the class ElfBinaryFile.
 */


/***************************************************************************/ /**
 * Dependencies.
 ******************************************************************************/

#include "boom_base/BinaryFile.h"

struct Elf32_Ehdr;
struct Elf32_Phdr;
struct Elf32_Shdr;
struct Elf32_Rel;
struct Elf32_Sym;

struct Translated_ElfSym;

class IBinaryImage;
class IBinarySymbolTable;
class QFile;


typedef struct
{
	ADDRESS uSymAddr; ///< Symbol native address
	DWord   iSymSize; ///< Size associated with symbol
} SymValue;


/**
 * File loader for loading 32 bit binary ELF files.
 */
class ElfLoader : public IFileLoader
{
	typedef std::map<ADDRESS, QString, std::less<ADDRESS> > RelocMap;

public:
	ElfLoader();
	virtual ~ElfLoader() override;

	/// @copydoc IFileLoader::getFormat
	LOAD_FMT getFormat() const override;

	/// @copydoc IFileLoader::getMachine
	MACHINE getMachine() const override;

	/// @copydoc IFileLoader::initialize
	void initialize(IBoomerang *sys) override;

	/// @copydoc IFileLoader::unload
	void unload() override;

	/// @copydoc IFileLoader::close
	void close() override;

	/// @copydoc IFileLoader::getImageBase
	ADDRESS getImageBase() override;

	/// @copydoc IFileLoader::getImageSize
	size_t getImageSize() override;

	/// @copydoc IFileLoader::isRelocationAt
	bool isRelocationAt(ADDRESS uNative) override;

	/// @copydoc IFileLoader::getMainEntryPoint
	/// (this should be a label in elf binaries generated by compilers).
	virtual ADDRESS getMainEntryPoint() override;

	/// @copydoc IFileLoader::getEntryPoint
	virtual ADDRESS getEntryPoint() override;

	/// @copydoc IFileLoader::canLoad
	int canLoad(QIODevice& fl) const override;

	/// @copydoc IFileLoader::loadFromMemory
	bool loadFromMemory(QByteArray& img) override;

private:
	/// @returns true if this file is a shared library file.
	bool isLibrary() const;

	/// Return a list of library names which the binary file depends on
	QStringList getDependencyList();

	// Apply relocations; important when compiled without -fPIC
	void applyRelocations();

	/// Not meant to be used externally, but sometimes you just have to have it.
	/// Like a replacement for elf_strptr()
	const char *getStrPtr(int idx, int offset); // Calc string pointer

	/// Reset internal state, except for those that keep track of which member
	/// we're up to
	void init();                                // Initialise most member variables
	int processElfFile();                       // Does most of the work

	/// FIXME: the below assumes a fixed delta
	ADDRESS nativeToHostAddress(ADDRESS uNative);

	/// Add appropriate symbols to the symbol table.
	/// @p secIndex is the section index of the symbol table.
	void addSyms(int secIndex);

	/// FIXME: this function is way off the rails. It seems to always overwrite the relocation entry with the 32 bit value
	/// from the symbol table. Totally invalid for SPARC, and most X86 relocations!
	/// So currently not called
	void addRelocsAsSyms(uint32_t secIndex);
	void setRelocInfo(SectionInfo *pSect);
	bool postLoad(void *handle) override; // Called after archive member loaded

	/// Search the .rel[a].plt section for an entry with symbol table index i.
	/// If found, return the native address of the associated PLT entry.
	/// A linear search will be needed. However, starting at offset i and searching backwards with wraparound should
	/// typically minimise the number of entries to search
	ADDRESS findRelPltOffset(int i);

	// Internal elf reading methods // TODO replace by Util::swapEndian

	/***************************************************************************/ /**
	 * \brief    Read a 2 or 4 byte quantity from host address (C pointer) p
	 * \note        Takes care of reading the correct endianness, set early on into m_elfEndianness
	 * \param    ps or pi: host pointer to the data
	 * \returns        An integer representing the data
	 ******************************************************************************/
	SWord elfRead2(const SWord *ps) const; // Read a short with endianness care
	DWord elfRead4(const DWord *pi) const; // Read an int with endianness care
	void elfWrite4(DWord *pi, DWord val);  // Write an int with endianness care

	/***************************************************************************/ /**
	 * \brief      Mark all imported symbols as such.
	 * This function relies on the fact that the symbols are sorted by address, and that Elf PLT
	 * entries have successive addresses beginning soon after m_PltMin
	 ******************************************************************************/
	void markImports();
	void processSymbol(Translated_ElfSym& sym, int e_type, int i);

private:
	size_t m_loadedImageSize;            ///< Size of image in bytes
	Byte *m_loadedImage       = nullptr; ///< Pointer to the loaded image
	
	Elf32_Ehdr* m_elfHeader   = nullptr; ///< ELF header
	Elf32_Phdr *m_programHdrs = nullptr; ///< Pointer to program headers
	Elf32_Shdr *m_sectionhdrs = nullptr; ///< Array of section header structs

	const char *m_strings = nullptr;     ///< Pointer to the string section
	bool m_bigEndian;                    ///< 1 = Big Endian

	// SymTab      m_Reloc;                 ///< Object to store the reloc syms
	const Elf32_Rel *m_relocSection  = nullptr; ///< Pointer to the relocation section
	const Elf32_Sym *m_symbolSection = nullptr; ///< Pointer to loaded symbol section

	bool m_relocHasAddend;                      ///< true if reloc table has addend
	ADDRESS m_lastAddr;                         ///< Save last address looked up
	int m_lastSize;                             ///< Size associated with that name
	ADDRESS m_pltMin;                           ///< Min address of PLT table
	ADDRESS m_pltMax;                           ///< Max address (1 past last) of PLT
	ADDRESS *m_importStubs = nullptr;           ///< An array of import stubs
	ADDRESS m_baseAddr;                         ///< Base image virtual address
	size_t m_imageSize;                         ///< total image size (bytes)
	ADDRESS m_firstExtern;                      ///< where the first extern will be placed
	ADDRESS m_nextExtern;                       ///< where the next extern will be placed
	int *m_shLink = nullptr;                    ///< pointer to array of sh_link values
	int *m_shInfo = nullptr;                    ///< pointer to array of sh_info values

	std::vector<struct SectionParam> m_elfSections;
	IBinaryImage *m_binaryImage;
	IBinarySymbolTable *m_symbols;
};
