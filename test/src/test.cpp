#include "sharedarray.h"

using namespace beautifulcode;

constexpr int constexpr_data[] = { 1, 2, 3 };

void test_slice()
{
	Slice<const int> slice({ 1, 2, 3 });
	assert(slice.length == 3);


	// --- test constexpr ---

	//	static_assert(Slice<int>(nullptr).length == 0, "!!!"); // TODO: VS2017 fails on this code!
	static_assert(Slice<int>().length == 0, "!!!");
	static_assert(Slice<const int>(constexpr_data, 10).length == 10, "!!!");
	static_assert(Slice<const int>(constexpr_data).length == 3, "!!!");
	static_assert(Slice<const int>(Slice<const int>(constexpr_data).slice(0, 1)).length == 1, "!!!");
	static_assert(Slice<const int>(constexpr_data).front() == 1, "!!!");
	static_assert(Slice<const int>(constexpr_data).back() == 3, "!!!");
	static_assert(Slice<const int>(constexpr_data) == Slice<const int>(constexpr_data), "!!!");
	//	static_assert(Slice<const int>(constexpr_data).eq(Slice<const int>(constexpr_data)), "!!!");
}

void test_string()
{
	const char *pstr = "hello";
	String str(pstr);
	assert(str.length == 5);

	// --- test STL ---
	std::string s("hello");
	String stl(s);
	assert(stl.eq("hello"));

}

void test_array()
{

	// --- test parsing ---

	assert(String("10").parse_int() == 10);
	assert(String("0x10").parse_int(true) == 16);
	assert(String("$10").parse_int(true) == 16);
	assert(String("b10").parse_int(true) == 2);
	assert(String("777").parse_int<8>() == 0x1FF);
	assert(String("1.5").parse_float() == 1.5);
	assert(String("1.5e+3").parse_float() == 1500.0);
	assert(String("1.5E-3").parse_float() == 0.0015);
}

void test_mutablestring()
{

	// --- test unicode transcoding ---

	MutableString<> ms1("hello");
	MutableString<> ms2(u8"日本語");
	MutableString<> ms3(L"日本語");
	MutableString<> ms4(u"日本語");
	MutableString<> ms5(U"日本語");

	MutableWString<> mu1("hello");
	MutableWString<> mu2(u8"日本語");
	MutableWString<> mu3(L"日本語");
	MutableWString<> mu4(u"日本語");
	MutableWString<> mu5(U"日本語");

	MutableDString<> mU1("hello");
	MutableDString<> mU2(u8"日本語");
	MutableDString<> mU3(L"日本語");
	MutableDString<> mU4(u"日本語");
	MutableDString<> mU5(U"日本語");

	MutableString<> test(Concat, "hello", mU3, 'a', U'日', L'ö', L"löewe");
	Array<wchar_t> test2(Concat, "hello", mU3, 'a', U'日', L'ö', L"löewe");
	Array<unsigned char> test3(Concat, "hello", mU3, 'a', U'日', L'ö', L"löewe");
	MutableDString<> test4 = test;

	MutableString<> test5(Sprintf, "hey %d %s %ls", 10, "joe", L"king");
	MutableWString<> test7(Sprintf, "hey %d %s %ls", 10, "joe", L"king");
#if defined(_MSC_VER)
	MutableString<> test6(Sprintf, L"hey %d %S %ls", 10, "joe", L"king");
	MutableWString<> test8(Sprintf, L"hey %d %S %ls", 10, "joe", L"king");
#else
	MutableString<> test6(Sprintf, L"hey %d %s %ls", 10, "joe", L"king");
	MutableWString<> test8(Sprintf, L"hey %d %s %ls", 10, "joe", L"king");
#endif

	test7.append(U'日', 'a', mU2);

	// --- test URL encode/decode ---
	test6.url_decode(String("something%2B!+%E6%97%A5%E6%9C%AC%E8%AA%9E+l%C3%B6ewe"));
	assert(test6.eq(u8"something+! 日本語 löewe"));
	test6.url_decode(Slice<const wchar_t>(L"something%2B!+%E6%97%A5%E6%9C%AC%E8%AA%9E+l%C3%B6ewe"));
	assert(test6.eq(u8"something+! 日本語 löewe"));
	test7.url_decode(String("something%2B!+%E6%97%A5%E6%9C%AC%E8%AA%9E+l%C3%B6ewe"));
	assert(test7.eq(u"something+! 日本語 löewe"));
	test7.url_decode(DString(U"something%2B!+%E6%97%A5%E6%9C%AC%E8%AA%9E+l%C3%B6ewe"));
	assert(test7.eq(u"something+! 日本語 löewe"));

	test6.url_encode(test7);
	assert(test6.eq("something%2B%21+%E6%97%A5%E6%9C%AC%E8%AA%9E+l%C3%B6ewe"));
}

void test_sharedarray()
{
	// -- test claiming single-ref ---
	SharedArray<int> sa = { 1, 2, 3 };
	int *p = sa.ptr;
	Array<int> claimed = sa.claim();
	assert(claimed.ptr == p && sa.ptr == nullptr);
}

void test_sharedstring()
{

}

void main()
{
	test_slice();
	test_string();
	test_array();
	test_mutablestring();
	test_sharedarray();
	test_sharedstring();
}
