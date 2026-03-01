#include <direct.h>
#include <fstream>
#include <stdlib.h>
#include <vector>

#include "pbg3/Pbg3Archive.hpp"
#include <munit.h>

static char *file_name_params[] = {"ascii.png", "th06logo.jpg", "text.anm", NULL};

static MunitParameterEnum test_params[] = {
    {"file_name", file_name_params},
    {NULL, NULL},
};

static MunitResult test_read_raw(const MunitParameter params[], void *user_data)
{
    Pbg3Archive archive;
    munit_assert_int(archive.Load("resources/KOUMAKYO_IN.dat"), !=, 0);

    // Read from PBG3
    i32 entryIdx = archive.FindEntry("th06logo.jpg");
    munit_assert_int(entryIdx, !=, -1);

    u32 size;
    u32 expectedCsum;
    u8 *entry = archive.ReadEntryRaw(&size, &expectedCsum, entryIdx);
    munit_assert_not_null(entry);

    return MUNIT_OK;
}

static MunitResult test_read_decompress(const MunitParameter params[], void *user_data)
{
    char *archiveNameRelative = "resources/KOUMAKYO_IN.dat";
    char *fileName = params[0].value;
    char archiveName[MAX_PATH] = {0};

    if (_fullpath(archiveName, archiveNameRelative, MAX_PATH) == NULL)
    {
        munit_errorf_ex(__FILE__, __LINE__, "Failed to canonicalize %s", archiveNameRelative);
        return MUNIT_FAIL;
    }

    Pbg3Archive archive;
    munit_assert_int(archive.Load(archiveName), !=, 0);

    // Read from PBG3.
    i32 entryIdx = archive.FindEntry(fileName);
    munit_assert_int(entryIdx, !=, -1);
    u8 *entry = archive.ReadDecompressEntry(entryIdx, fileName);
    munit_assert_not_null(entry);
    u32 entrySize = archive.GetEntrySize(entryIdx);
    munit_assert_int(entrySize, !=, 0);

    // Create a temporary directory to work from.
    char tmpNameStr[MAX_PATH] = {0};
    tmpnam(tmpNameStr);
    // Remove trailing dot.
    for (int i = 0; i < MAX_PATH; i++)
    {
        if (i == 0)
        {
            if (tmpNameStr[i - 1] == '.')
            {
                tmpNameStr[i - 1] = '\0';
            }
            break;
        }
    }

    char tmpPathCStr[MAX_PATH] = {0};
    DWORD tmpPathSize = GetTempPathA(MAX_PATH, tmpPathCStr);
    munit_assert_int(tmpPathSize, !=, 0);
    std::string tmpPath(tmpPathCStr, tmpPathSize);
    if (tmpNameStr[0] != '\\')
    {
        tmpPath += "\\";
    }
    tmpPath += tmpNameStr;
    _mkdir(tmpPath.c_str());

    munit_logf_ex(MUNIT_LOG_INFO, __FILE__, __LINE__, "Temporary path: %s", tmpPath.c_str());

    // Write decompressed file to disk. Useful for debugging.
    std::string actualPath(tmpPath);
    actualPath += "/";
    actualPath += fileName;
    actualPath += ".actual";
    std::ofstream outfile(actualPath.c_str(), std::ios::out | std::ios::binary);
    outfile.write((char *)entry, entrySize);

    // Extract using reference implementation.
    std::string cmd("cd /d ");
    cmd += tmpPath;
    cmd += " && thdat -x 6 ";
    cmd += archiveName;
    cmd += " ";
    cmd += params[0].value;
    munit_logf_ex(MUNIT_LOG_INFO, __FILE__, __LINE__, "Running %s", cmd.c_str());
    munit_assert_int(system(cmd.c_str()), !=, -1);

    // Read reference extraction
    std::string referencePath(tmpPath);
    referencePath += "/";
    referencePath += params[0].value;
    std::ifstream infile(referencePath.c_str(), std::ios::binary);
    std::vector<char> buffer(std::istreambuf_iterator<char>(infile), std::istreambuf_iterator<char>());
    u8 *reference = (u8 *)&buffer[0];
    u32 referenceSize = buffer.size();

    // Ensure they are equal.
    munit_assert_int(entrySize, ==, referenceSize);
    munit_assert_memory_equal(entrySize, entry, reference);
    return MUNIT_OK;
}

static MunitTest pbg3archives_test_suite_tests[] = {
    {"/read_decompress", test_read_decompress, NULL, NULL, MUNIT_TEST_OPTION_NONE, test_params},
    {"/read_raw", test_read_raw, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
    /* Mark the end of the array with an entry where the test
     * function is NULL */
    {NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL}};
