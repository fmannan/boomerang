int main(int argc, char *argv[]);


/** address: 0x00001bc0 */
int main(int argc, char *argv[])
{
    int g3; 		// r3
    int g3_1; 		// r3
    struct stat local0; 		// m[g1 - 112]
    int local1; 		// m[g1 - 108]
    int local10; 		// m[g1 - 88]
    int local11; 		// m[g1 - 80]
    int local12; 		// m[g1 - 72]
    unsigned short local2; 		// m[g1 - 104]
    unsigned short local3; 		// m[g1 - 102]
    int local4; 		// m[g1 - 100]
    int local5; 		// m[g1 - 96]
    int local6; 		// m[g1 - 92]
    int local7; 		// m[g1 - 64]
    int local8; 		// m[g1 - 48]
    int local9; 		// m[g1 - 56]

    g3 = *(argv + 4);
    g3_1 = stat(g3, &local0);
    printf("res: %i\n", g3_1);
    printf("dev: %i\n", local0);
    printf("ino: %i\n", local1);
    printf("mode: %i\n", (local2));
    printf("nlink: %i\n", (local3));
    printf("uid: %i\n", local4);
    printf("gid: %i\n", local5);
    printf("rdev: %i\n", local6);
    printf("size: %i\n", local7);
    printf("blksize: %i\n", local8);
    printf("blocks: %i\n", local9);
    printf("atime: %i\n", local10);
    printf("mtime: %i\n", local11);
    printf("ctime: %i\n", local12);
    return 0;
}

