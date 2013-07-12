/* This program is a hack.
   It should really write to an AST and serialize it, rather than do
   file telling and mutation!

   Should rewrite in Haskell.
*/
#include <iostream>
#include <fstream>

#include <cstdio>

// Mac OS specific.
#include <libkern/OSByteOrder.h>

using namespace std;

// Copied from MovieFormat.h
enum {
  kDataRate144ModemRate         = 1400,
  kDataRate288ModemRate         = 2800,
  kDataRateISDNRate             = 5600,
  kDataRateDualISDNRate         = 11200,
  kDataRate256kbpsRate          = 25600,
  kDataRate384kbpsRate          = 38400,
  kDataRate512kbpsRate          = 51200,
  kDataRate768kbpsRate          = 76800,
  kDataRate1MbpsRate            = 100000,
  kDataRateT1Rate               = 150000,
  kDataRateInfiniteRate         = 0x7FFFFFFF,
  kDataRateDefaultIfNotSet      = kDataRate512kbpsRate
};

const string suffix56k = "_56k_S.mov";
const string suffix256k = "_256k_S.mov";
const string suffix512k = "_512k_S.mov";
const string suffixT1 = "_T1_S.mov";

const string pathSep = "/";
const string movSuffix = ".mov";

// writeHeader writes the atom header returning the header offset for
// each atom being written
auto writeHeader(FILE *outputFile,
                 const uint32_t atomType) -> off_t
{
  const uint32_t atomSize = OSSwapHostToBigInt32('patc');

  // ftello is identical to ftell but returns an off_t which can be a 64-bit type
  const auto headerOffset = ftello(outputFile);

  // will be patched later
  fwrite(&atomSize, sizeof(atomSize), 1, outputFile);

  fwrite(&atomType, sizeof(atomType), 1, outputFile);

  return headerOffset;
}

// patchHeader updates the atom size correctly after all the atom data
// has been written
auto patchHeader(FILE *outputFile,
                 const off_t headerOffset) -> void
{
  const auto saveOffset = ftello(outputFile);
  const uint32_t atomSize = OSSwapHostToBigInt32(saveOffset - headerOffset);
  fseeko(outputFile, headerOffset, SEEK_SET);

  fwrite(&atomSize, sizeof(atomSize), 1, outputFile);
  fseeko(outputFile, saveOffset, SEEK_SET);
}

// write the data rate atom 'rmdr'
auto write_rdrf(FILE *outputFile,
                const string &path) -> void
{
  const auto pathSize = path.length() + 1; // include NUL
  const uint32_t flags = 0;
  const auto dataRefType = OSSwapHostToBigInt32('url ');
  const auto dataRefSize = OSSwapHostToBigInt32(pathSize);
  const auto headerOffset = writeHeader(outputFile,
                                        OSSwapHostToBigInt32('rdrf'));
  fwrite(&flags, sizeof(flags), 1, outputFile);
  fwrite(&dataRefType, sizeof(dataRefType), 1, outputFile);
  fwrite(&dataRefSize, sizeof(dataRefSize), 1, outputFile);
  fwrite(path.c_str(), pathSize, 1, outputFile);
  patchHeader(outputFile, headerOffset);
}

// write the data reference atom 'rdrf'
auto write_rmdr(FILE *outputFile,
                const uint32_t dataRate) -> void
{
  const auto headerOffset = writeHeader(outputFile,
                                        OSSwapHostToBigInt32('rmdr'));

  const uint32_t flags = 0;
  fwrite(&flags, sizeof(flags), 1, outputFile);

  fwrite(&dataRate, sizeof(dataRate), 1, outputFile);

  patchHeader(outputFile, headerOffset);
}

// for each reference, create the appropriate 'rmda'
// in each reference write a data reference atom 'rdrf' and a data rate atom 'rmdr'
auto write_rmda(FILE *outputFile,
                const string &path,
                const uint32_t dataRate) -> void
{
  const auto headerOffset = writeHeader(outputFile,
                                        OSSwapHostToBigInt32('rmda'));
  write_rdrf(outputFile, path);
  if (dataRate != 0) {
    write_rmdr(outputFile, OSSwapHostToBigInt32(dataRate));
  }
  patchHeader(outputFile, headerOffset);
}

auto alternatePath(const string &server,
                   const string &path,
                   const string &name,
                   const string &suffix) -> string
{
  return server + pathSep + path + pathSep + name + pathSep + name + suffix;
}

auto soundPath(const string &server,
               const string &path,
               const string &name,
               const string &suffix) -> string
{
  return server + pathSep + path + pathSep + name + suffix;
}

// write the 'rmra' and add each Reference Movie Descriptor Atom 'rmda'
// NOTE: The order DOES matter here  -- the last one that passes all of
// its checks (data rate, version check) wins.
auto write_rmra(FILE *outputFile,
                const bool isMovie,
                const string &server,
                const string &path,
                const string &name) -> void
{
  const auto headerOffset = writeHeader(outputFile,
                                        OSSwapHostToBigInt32('rmra'));

  if (isMovie) {
    // Write out the four alternates.
    write_rmda(outputFile,
               alternatePath(server,
                             path,
                             name,
                             suffix56k),
               kDataRateISDNRate);

    write_rmda(outputFile,
               alternatePath(server,
                             path,
                             name,
                             suffix256k),
               kDataRate256kbpsRate);

    write_rmda(outputFile,
               alternatePath(server,
                           path,
                           name,
                           suffix512k),
               kDataRate512kbpsRate);

    write_rmda(outputFile,
               alternatePath(server,
                             path,
                             name,
                             suffixT1),
               kDataRateT1Rate);
  }
  else {
    // Sound
    write_rmda(outputFile,
               soundPath(server,
                         path,
                         name,
                         movSuffix),
               0);
  }

  patchHeader(outputFile, headerOffset);
}

// create the reference movie - write the 'moov' header and populate
// the 'rmra' atom
auto write_moov(FILE *outputFile,
                const bool isMovie,
                const string &server,
                const string &path,
                const string &name) -> void {
  const auto headerOffset = writeHeader(outputFile,
                                        OSSwapHostToBigInt32('moov'));
  write_rmra(outputFile, isMovie, server, path, name);
  patchHeader(outputFile, headerOffset);
}

void usage(const string &prog)
{
  cerr << "Usage: " << prog << "mov|snd server path name outputDir" << endl;
  ::exit(1);
}

auto main(int argc, const char **argv) -> int {
  if (argc != 6) {
    usage(argv[0]);
  }

  const string mediaType = argv[1];

  bool isMovie = false;
  if (mediaType == "mov") {
    isMovie = true;
  }
  else if (mediaType == "snd") {
    isMovie = false;
  }
  else {
    usage(argv[0]);
  }

  const string server = argv[2];
  const string path = argv[3];
  const string name = argv[4];
  const string outputDir = argv[5];

  const string outputPath = outputDir + pathSep + name + movSuffix;

  FILE *outputFile = fopen(outputPath.c_str(), "w");
  if (outputFile == NULL) {
    perror(outputPath.c_str());
    exit(1);
  }

  write_moov(outputFile, isMovie, server, path, name);
  fclose(outputFile);
}
