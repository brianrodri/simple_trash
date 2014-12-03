#include <algorithm>
#include <ctype.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include "trash_node.h"

using namespace std;
using namespace boost;
using namespace boost::filesystem;

extern int g_argc;
extern char ** g_argv;



trash_node::trash_node(const directory_entry& entry)
{
	string fname(entry.path().filename().string()), line;
	istringstream is(fname);
	ifstream f(entry.path().c_str());

	currpath = (TRASH_PATH / entry.path()).filename();

	if (!std::all_of(fname.begin(), fname.end(), ::isdigit) || !f.good() || !exists(currpath) || !is_regular_file(entry.path())) {
		cerr << g_argv[0] << ": `~/.trash' seems to be corrupted. Please run `trash --empty' to enter a clean trash-system state." << endl;
		exit(1);
	}

	is >> id;
	infopath = entry.path();
	getline(f, line);
	prevpath = line;
	f.close();
}

bool
trash_node::operator<(const trash_node& rhs) const
{
	return id < rhs.id;
}

bool
trash_node::operator<(const size_t& rhs) const
{
	return id < rhs;
}

void
trash_node::remove()
{
	boost::filesystem::remove(infopath);
	boost::filesystem::remove(currpath);
}

void
trash_node::restore()
{
	if (exists(prevpath)) {
		char response;

		cout << g_argv[0] << ": Overwrite `" << prevpath.string() << "'? [Y/n] ";
		cin >> response;

		if (response != 'Y' && response != 'y') {
			return;
		}
	}

	boost::filesystem::remove(infopath);
	boost::filesystem::rename(currpath, prevpath);
}

string
trash_node::info() const
{
	ostringstream os;

	os << "{" << id << ":" << prevpath << "}";
	return os.str();
}

vector <trash_node>
build_trash_bin()
{
	vector <trash_node> trash_bin;

	try {
		create_directories(TRASH_INFO_PATH);

	} catch (const std::exception& ex) {
		cerr << g_argv[0] << ": `~/.trash' seems to be corrupted. Please run `trash --empty' to enter a clean trash-system state." << endl;
		exit(1);
	}

	trash_bin.assign(directory_iterator(TRASH_INFO_PATH), directory_iterator());
	sort(trash_bin.begin(), trash_bin.end());
	return trash_bin;
}

size_t
find_free_id(const vector <trash_node>& bin)
{
	if (bin.empty()) {
		return (size_t) 0;
	}

	size_t lo, hi;
	lo = 0;
	hi = bin.size() - 1;

	while (true) {
		if (lo >= hi) {
			if (hi < bin[hi].id) {
				return hi;

			} else {
				return (size_t) bin.size();
			}
		}

		size_t mid = (lo / 2) + (hi / 2) + (lo & hi & 1);

		if (mid < bin[mid].id) {
			hi = mid;

		} else {
			lo = mid + 1;
		}
	}
}
