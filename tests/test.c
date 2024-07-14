#include <assert.h>
#define NSTRINGS_IMPLEMENTATION
#define NSTRINGS_MAIN
#include "nstrings.h"

int
Main(int argc, string* argv)
{
	{ /* Test Equals() */
		assert(Equals$("here i am foobar 124",$("here i am foobar 124")));
		assert(!Equals$("here i am foobar 124",$("here i am foobar 12")));
	}

	{ /* Test Concat() */
		char buf[128];
		string s = Concat(buf, $("foo"), $("bar"));
		assert(Equals$("foobar", s));
	}

	{ /* Test Slice() */
		string s = $("foobar");
		string x = Slice(s, 1, 4);
		assert(Equals$("oob", x));
	}

	{ /* Test HasPrefix() */
		string s = $("foobar");
		assert(HasPrefix$("foo", s));
		assert(!HasPrefix$("oobar", s));
	}

	{ /* Test IndexOf() */
		string s = $("foobar");
		assert(IndexOf(s, 'b') == 3);
		assert(IndexOf(s, 'f') == 0);
		assert(IndexOf(s, 'r') == 5);
		assert(IndexOf(s, 't') < 0);
	}

	Println$("%S", "passed.");
	return 0;
}
