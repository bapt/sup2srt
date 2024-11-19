#include <iostream>
#include <string>
#include <unistd.h>
#include <cstdlib>
#include "pgs/pgsParser.h"
#include "srtUtil.h"
#include "mkvUtil.h"
#include "sup2srtConfig.h"

std::string usage = "Usage: sup2srt [-h] [-vST] [-t track] [-l language] [-o output] input\n\n  -h:             Display this help menu\n  -v:             Display program Version\n  -V:             Verbose - output srt as it's being written\n  -T:             Dump TIFF images extracted from the PGS file to disk\n  -S:             Write SUP file extracted from video file to disk (only used when an mkv or m2ts is input)\n  -t track:       Select track to extract from video file. Can be multiple tracks separated by a comma (must be used when an mkv or m2ts is input)\n  -l language:    Select Tesseract language according to ISO 639-2/T. If not specified and an mkv or m2ts is input, the program will try to use the track metadata. Must be used when a binary SUP file is input\n  -o:             Output file (if not specified the file will be output to the same dir as the input).\n  input:          file to parse SUP stream. Can either be a binary SUP file or a video file. If the video file doesn't end in .mkv or .m2ts, it will assume it is a binary SUP file.\n";
std::string input;
std::string output = std::string("-1");
std::string language = std::string("-1");
std::vector<unsigned int> tracks;
std::string version = std::to_string(sup2srt_VERSION_MAJOR) + "." + std::to_string(sup2srt_VERSION_MINOR) + "." + std::to_string(sup2srt_VERSION_PATCH);
//int track = -1;
bool verbose = false;
bool dumpTIFF = false;
bool dumpSUP = false;
bool mkv = false;


void parseArgs(int argc, char** argv)
{
	extern char *optarg;
	extern int optind;
	int o;
	while ((o = getopt (argc, argv, "hTSl:o:t:vV")) != -1)
	{
		switch(o)
		{

			case 'h':
			{
				std::cout << usage << std::endl;
				exit(0);
			}
			case 'v':
			{
				std::cout << "sup2srt version " << version << std::endl;
				exit(0);
			}
			case 'T':
			{
				dumpTIFF = true;
				break;
			}
			case 'S':
			{
				dumpSUP = true;
				break;
			}
			case 'l':
			{
				language = std::string(optarg);
				break;
			}
			case 'o':
			{
				output = std::string(optarg);
				break;
			}
			case 't':
			{
				//track = atoi(optarg);
				std::string trackString(optarg);
				tracks = mkvUtil::parseTracks(trackString);
				break;
			}
			case 'V':
			{
				verbose = true;
				break;
			}
			case '?':
			{
				std::cerr << "Did not understand argument: " << o <<std::endl;
				std::cerr << usage;
				exit(1);
			}
		}
	}
	if ((optind+1) != argc)
	{
		std::cerr << "Please supply an input file" << std::endl;
		std::cerr << usage;
		exit(1);
	}
	else
	{
		input = std::string(argv[optind]);
		if(input.find('.') < input.length())
		{
			mkv = input.substr(input.find_last_of('.') + 1).compare("mkv") == 0;
			mkv = mkv || input.substr(input.find_last_of('.') + 1).compare("m2ts") == 0;
		}
	}
	if(mkv && tracks.size() == 0)
	{
		//std::cerr << "Please supply a track index if you input an mkv file" << std::endl;
		//std::cerr << usage;
		//exit(1);
		std::cout << "No tracks provided. Attempting to find PGS tracks automatically..." << std::endl;
		tracks = mkvUtil::findAllPGSTracks(input);
	}
	if(language.compare("-1") == 0 && !mkv)
	{
		std::cerr << "Please supply a language according to ISO 639-2/T" << std::endl;
		std::cerr << usage;
		exit(1);
	}
	if(output.compare("-1") == 0 && !mkv)
	{
		if(input.find(".") < input.length() && input.substr(input.find_last_of('.') + 1).compare("sup") == 0)
		{
			output = input.substr(0, input.find_last_of('.'));
		}
		else
		{
			output = input;
		}
		output += "." + language + ".srt";
	}
}

int main(int argc, char** argv)
{
	parseArgs(argc, argv);
	if(mkv)
	{
		std::string basename = input.substr(0, input.find_last_of('.')+1);
		std::cout << "Extracting SUP stream";
		std::cout << (tracks.size() > 1 ? "s" : "");
		std::cout << " from video file..." << std::endl;
		std::vector<supStream> streams = mkvUtil::extractSelectMKVsup(input, tracks);
		std::cout << "Stream";
		std::cout << (tracks.size() > 1 ? "s" : "");
		std::cout << " extracted" << std::endl;
		for(int i = 0; i < streams.size(); i++)
		{
			std::string lang = language.compare("-1") == 0 ? streams[i].language : language;
			std::stringstream pgs;
			pgs << streams[i].data;
			pgsParser pgsp(&pgs);
			if(dumpSUP)
			{
				std::ofstream file;
				std::string filename = basename + std::to_string(streams[i].track) + "." + lang + ".sup";
				file.open(filename, std::ifstream::binary);
				file << streams[i].data;
				file.close();
				std::cout << "SUP file written to: " + filename << std::endl;
			}
			if(dumpTIFF)
			{
				std::string path = basename + std::to_string(streams[i].track) + "." + lang + ".sup.TIFFs";;
				std::cout << "TIFF files being written to: " + path << std::endl;
				pgsp.dumpTIFFs(path, false);
				std::cout << "TIFF files have been written" << std::endl;
			}
			std::cout << "Starting OCR on extracted images..." << std::endl;
			std::string filename = basename + std::to_string(streams[i].track) + "." + lang + ".srt";
			srtUtil::pgsToSRTFile(&pgsp, filename.c_str(), lang.c_str(), verbose);
			std::cout << "OCR finished and SRT written to: " + filename << std::endl;
		}
	}
	else
	{
		std::stringstream pgs;
		std::cout << "Reading SUP from file..." << std::endl;
		std::ifstream file(input, std::ifstream::binary);
		pgs << file.rdbuf();
		file.close();
		std::cout << "SUP file has been read" << std::endl;
		pgsParser pgsp(&pgs);
		if(dumpTIFF)
		{
			std::string path = input + ".TIFFs";;
			std::cout << "TIFF files being written to: " + path << std::endl;
			pgsp.dumpTIFFs(path, false);
			std::cout << "TIFF files have been written" << std::endl;
		}
		std::cout << "Starting OCR on extracted images..." << std::endl;
		srtUtil::pgsToSRTFile(&pgsp, output.c_str(), language.c_str(), verbose);
		std::cout << "OCR finished and SRT written to: " + output << std::endl;
	}
    return 0;
}
