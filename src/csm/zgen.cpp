
/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cmath>

#include <sys/time.h>

#define HTMLTAGS 0

static const char * USAGE_TXT =
 "\nzgen generates files of a given size, with a given compression ratio."
 "\n\nThe destination file consists of a large number of blocks"
 "\nof given size. Some part of block is a sequence of given length"
 "\nmade up of one randomly choosen character, and the rest is"
 "\neither generated with random number generator or taken from"
 "\noptionaly specified file."
 "\n\nThe number of filler characterss and the size of resulting file"
 "\nare configurable. If the source file is specified and the resulting"
 "\nfile is bigger than the source file, then the tool starts reading"
 "\nthe source file nonce again from the start. This is enough to fool"
 "\nmost compression engines with any reasonable sized file. Any small"
 "\njpeg file will do. Tested on 64kb sized file."
 "\n\nNote that the best usage of source file is to simulate text content"
 "\nwith high compressibility using some average text file as a source."
 "\n\nUsage:"
 "\n\nzgen <dest file> <block size> <num of spaces> <final fsize> [source file]"
 "\n\nWhere:"
 "\n<dest file>   - output file name"
 "\n<block size>  - the size of block to work on, 16384 will do."
 "\n<filler size> - the number of bytes for easily compressable filler"
 "\n                sections to use in each block."
 "\n                Should be equal or less than block size. If"
 "\n                you need your file to be compressible to 75%,"
 "\n                replace at least 25% of each block with filler."
 "\n<final fsize> - the size of file we want to get. It is"
 "\n                rounded to the nearest number divisable"
 "\n                by block size, i.e. 2000 fsize with 1024 block"
 "\n                is rounded to 2048"
 "\n[source file] - some essentially uncompressible file, i.e. jpg"
 "\n                or archive. It is used as a source of content"
 "\n                instead of number generator if specified)";

using namespace std;

int main(int argc, char *argv[])
{
    if (argc < 5 || argc > 6)
    {
        cout << USAGE_TXT << endl;
	return 1;
    }

    int i = 1;
    const char * ofname  = argv[i++];
    const unsigned int bsize      = atoi(argv[i++]);
    const unsigned int nspaces    = atoi(argv[i++]);
    unsigned int ofsize           = atoi(argv[i++]);

    if (ofsize < bsize)
    {
        cout << "Warning: the size of target file is lower"
	        "than blocks size. Check the order of arguments" << endl;
    }
    
    char * ifname = 0;
    
    ifstream infile;
    bool infileSpecified = false;
    if (argc == 6) // source file is specified
    {
         infileSpecified = true;
         ifname = argv[i++];
         infile.open(ifname);
         if (! infile.good())
         {
            cout << "Error: can not read file " << ifname << endl;
            return 1;
         }
    }

    ofstream outfile(ofname);
    if (! outfile.good())
    {
        cout << "Error: can not open output file " << ifname
	     << " for writing" <<endl;
        return 1;
    }
    
    if (nspaces > bsize)
    {
        cout << "Error: num of spaces should be less or equal to block size" << endl;
        return 1;
    }

    // we want ofsize to be divisable by bsize, makes our life easier
    if (ofsize % bsize != 0)
    {
        ofsize += bsize - (ofsize % bsize);
    }

    int bytesWritten = 0;
    const int readBytes = bsize - nspaces;
    char * buff = new char[bsize];
    
    timeval tim;
    gettimeofday(&tim, NULL);
    srand((unsigned)(tim.tv_sec + tim.tv_usec / 1000));

#if HTMLTAGS
    outfile << "<html><body>\n";
#endif

    while (bytesWritten < ofsize)
    {
        // fill all buffer with one character
        memset(buff, rand() % 255, bsize);

	int initPos = nspaces == 0 ? 0 : rand() % (bsize - readBytes);
        int buffPos = initPos;

        while(buffPos < readBytes + initPos)
	{
	    if (infileSpecified)
	    {
                if (infile.eof())
                {
	            infile.clear();
		    infile.seekg(0, ios::beg);
	        }

                if (! infile.good())
	        {
	            cout << "Error: problem reading file" << ifname << endl;
		    return 1;
	        }
	    
	        infile.read(buff, readBytes-buffPos);
	        buffPos += infile.gcount();
	    } else
	    {
                // avoid inserting control characters
	        char ch =
#if HTMLTAGS
		  (unsigned char) (32 + (rand() % (256-32)));
#else
	          (unsigned char)(0.5 + (rand() / (0.1 + (RAND_MAX >> 8))));
#endif

#if HTMLTAGS
                // avoid messing with html tags and escape characters
                if (ch == '<' || ch == '>' || ch == '&' || ch=='"' || ch==127)
	        {
	            ch = ' ';
	        }
#endif
	        buff[buffPos++] = ch;
	    }
	}

#if HTMLTAGS
        outfile << "<p>";
#endif
	outfile.write(buff, bsize);
#if HTMLTAGS
        outfile << "</p>\n";
#endif
	if (outfile.bad())
	{
	    cout << "Error writing to the output file " << ofname << endl;
	    return 1;
	}
        bytesWritten += bsize;
    }

    delete[] buff;
    
    if (infileSpecified)
    {
        infile.close();
    }
#if HTMLTAGS
    outfile << "</body></html>" << endl;
#endif
    outfile.close();
    
    return 0;
}
