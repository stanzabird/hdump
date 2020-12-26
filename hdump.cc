#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

#include <config.h>
#include <getopt.h>

#include <sys/types.h>
#include <sys/ioctl.h>
#include <unistd.h>


// getting the terminal width...
int term_cols, term_lines;
int
get_term_size (void)
{
  struct winsize ts;
  ioctl(STDIN_FILENO, TIOCGWINSZ, &ts);
  term_cols = ts.ws_col;
  term_lines = ts.ws_row;
  return 0;
}

// manually specify the width...
int width{0};


void
hdump_istream(std::istream& is, int width) {
  using char_t = unsigned char;
  const auto n = (width - 1) / 4;  
  is >> std::noskipws;
  
  while (is)
    {
      std::vector<char_t> chars;
      
      for (auto i = n; i > 0; i--) {
	char_t c;
	is >> c;
	if (is)
	  chars.push_back(c);
      }

      std::ostringstream oss_hex;
      for (auto c : chars)
	oss_hex << ' ' << std::hex << std::setw(2) << std::setfill('0') << (unsigned int) c;
      std::ostringstream oss_printable;
      for (auto c : chars)
	if (isprint(c))
	  oss_printable.put( (char) c );
	else
	  oss_printable.put( '.' );
      std::ostringstream oss_white;
      for (auto i = 3*(n-chars.size())+1; i > 0; i--)
	oss_white << ' ';
      
      std::cout << oss_hex.str() << oss_white.str() << oss_printable.str() << std::endl;
    }
}
void
hdump(std::string name) {
  auto i = term_cols;
  if (name.empty()) {
    i = 80;
    if (width) i = width;
    hdump_istream(std::cin,i);
  }
  else {
    std::ifstream ifs{name};
    if (width) i = width;
    hdump_istream(ifs,i);
  }
}


void
print_usage()
{
  std::cout
    << "Use: hdump [options] FILE...\n"
    << "\n"
    << "Available options:\n"
    << "  -h, --help               Display this help.\n"
    << "  -w NUM, --width=NUM      Set terminal width to NUM. (todo)\n"
    << "\n"
    ;
}

int
main(int argc, char* argv[])
{
  get_term_size();
  
  static struct option long_options[] =
    {
      {"width",   required_argument, 0, 'w'},
      {"help",    no_argument,       0, 'h'},
      {0, 0, 0, 0}
    };

  while (1)
    {
      int option_index = 0, c;
      
      c = getopt_long (argc, argv, "w:h", long_options, &option_index);
      
      if (c == -1) break; // end of options

      switch (c)
	{
	case 'w':		// --width,-w
	  width = std::stoi(optarg);
	  break;
	case 'h':		// --help,-h
	  {
	    print_usage();
	    return 1;
	  }
	  break;
	case '?':
	  print_usage();
	  return 1; /* getopt already printed a message */
	default:
	  abort();
	}
    }

  // process non-options
  if (optind < argc) {
    while (optind < argc)
      hdump(argv[optind++]);
  }
  else
    hdump({});
  
  return 0;
}
