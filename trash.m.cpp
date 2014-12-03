#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <vector>
#include <sys/ioctl.h>
#include "trash_node.h"

using namespace std;
using namespace boost;
using namespace boost::filesystem;

int g_argc;
char ** g_argv;

void print_table(const vector <string>& v);



int
main(int argc, char ** argv)
{
	// Easier to make command line arguments global
	g_argc = argc;
	g_argv = argv;

	if (g_argc == 1) {
		cerr << g_argv[0] << ": missing operand" << endl
			<< "Try `" << g_argv[0] << " --help' for more information." << endl;
		return 1;
	}

	string opt(g_argv[1]);

	if (opt == "-h" || opt == "--help") {
		cout << "Usage: " << g_argv[0] << " [opt] [files]..." << endl
			<< "    -a, --all                 Restores all items from the trash-system." << endl
			<< "    -e, --empty               Permanently removes all items from the trash-system." << endl
			<< "    -l, --list                Print contents of trash-system and the node IDs of its items." << endl
			<< "    -p, --purge [IDs]...      Permanently removes the passed in node IDs from the trash-system." << endl
			<< "    -r, --restore [IDs]...    Restores the passed in node IDs from the trash-system." << endl
			<< "    -v, --verbose [files]...  Place files as items to be managed by trash-system and prints out their respective IDs (-1 on failures)." << endl
			<< "    [files]...                Place files as items to be managed by trash-system." << endl;
		return 0;

	} else if (opt == "-a" || opt == "--all") {
		vector <trash_node> trash_bin(build_trash_bin());
		for_each(trash_bin.begin(), trash_bin.end(), [] (trash_node& itm) { itm.restore(); } );
		return 0;

	} else if (opt == "-e" || opt == "--empty") {
		remove_all(TRASH_PATH);
		return 0;

	} else if (opt == "-l" || opt == "--list") {
		vector <trash_node> trash_bin(build_trash_bin());

		if (!trash_bin.empty()) {
			vector <string> elements;

			for_each(trash_bin.begin(), trash_bin.end(), [&] (trash_node& itm) {
				elements.push_back(itm.info());
			} );

			print_table(elements);
		}

		return 0;

	} else if (opt == "-p" || opt == "--purge") {
		vector <trash_node> trash_bin(build_trash_bin());

		for_each(g_argv + 2, g_argv + g_argc, [&] (const char * id_str) {
			istringstream is(id_str);
			size_t id;

			is >> id;
			auto pos = lower_bound(trash_bin.begin(), trash_bin.end(), id);

			if (pos != trash_bin.end() && pos->id == id) {
				pos->remove();
				trash_bin.erase(pos);

			} else {
				cerr << g_argv[0] << ": cannot find node id `" << id_str << "' in trash-system" << endl;
			}
		} );

		return 0;

	} else if (opt == "-r" || opt == "--restore") {
		vector <trash_node> trash_bin(build_trash_bin());

		for_each(g_argv + 2, g_argv + g_argc, [&] (const char * id_str) {
			istringstream is(id_str);
			size_t id;

			is >> id;
			auto pos = lower_bound(trash_bin.begin(), trash_bin.end(), id);

			if (pos != trash_bin.end() && pos->id == id) {
				pos->restore();
				trash_bin.erase(pos);

			} else {
				cerr << g_argv[0] << ": cannot find node id `" << id_str << "' in trash-system" << endl;
			}
		} );

		return 0;

	} else { // throw in trash
		vector <trash_node> trash_bin(build_trash_bin());
		vector <string> elements;
		char off;

		if (opt == "-v" || opt == "--verbose") {
			off = 2;

		} else {
			off = 1;
		}

		for_each(g_argv + off, g_argv + g_argc, [&] (const char * fname) {
			path p = path(fname);

			if (exists(p)) {
				size_t new_id = find_free_id(trash_bin);
				p = complete(p);
				ostringstream os; os << new_id;
				path infopath = TRASH_INFO_PATH / path(os.str());
				path newpath = TRASH_PATH / path(os.str());
				ofstream infof(infopath.c_str());

				if (infof.good()) {
					infof << p.string() << endl;
					infof.close();
					rename(p, newpath);
	
					trash_bin.insert(lower_bound(trash_bin.begin(), trash_bin.end(),
						new_id), trash_node(new_id, p, newpath, infopath));
					elements.push_back(os.str());

				} else {
					cerr << g_argv[0] << ": `~/.trash' seems to be corrupted. Please run `trash --empty' to enter a clean trash-system state." << endl;
					exit(1);
				}

			} else {
				cerr << g_argv[0] << ": cannot trash `" << fname << "': No such file or directory" << endl;
				elements.push_back("-1");
			}
		} );

		if (off == 2) {
			print_table(elements);
		}

		return 0;
	}
}

void
print_table(const std::vector <string>& v)
{
	if (v.empty()) {
		return;
	}

	size_t maxlen = std::max_element(v.begin(), v.end(), [](const string& lhs, const string& rhs) {
		return lhs.length() < rhs.length(); } )->length(), columns;

	struct winsize w;
	ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
	columns = w.ws_col;

	if (maxlen < columns / 2) {
		size_t cols = columns / maxlen, cur = 0;

		for_each(v.begin(), v.end(), [&](const string& str) {
			cout << left << setw(maxlen) << str << setw(1);

			if (++cur < cols) {
				cout << "  ";

			} else {
				cur = 0;
				cout << endl;
			}
		} );

		if (cur) {
			cout << endl;
		}

	} else {
		for_each(v.begin(), v.end(), [](const string& str) { cout << str << endl; } );
	}
}
