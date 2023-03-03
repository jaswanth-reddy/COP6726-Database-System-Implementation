#include "gTest.h"

TEST(DBFile, metaInfoToStr) {
	DBFile file = DBFile();
	OrderMaker sortord = OrderMaker();
	FileMetaInfo meta = {
		sorted,
		100,
		sortord
	};
	string str = "sorted\n100\n0";
	ASSERT_EQ(str, file.metaInfoToStr(meta));
}

TEST(DBFile, readMetaInfo) {
	DBFile dbfile = DBFile();
	OrderMaker sortord = OrderMaker();
	FileMetaInfo meta = {
		sorted,
		100,
		sortord
	};
	
	string str = dbfile.metaInfoToStr(meta);
	string filest = "test_meta";
	dbfile.writeMetaInfo(filest, meta);
	FileMetaInfo read = dbfile.readMetaInfo(filest);
	ASSERT_EQ(str, dbfile.metaInfoToStr(read));

}

TEST(DBFile, Open) {
	DBFile dbfile = DBFile();
	OrderMaker sortord = OrderMaker();
	FileMetaInfo meta = {
		heap,
		100,
		sortord
	};
	
	string str = dbfile.metaInfoToStr(meta);
	string filest = "test_open.meta";
	dbfile.writeMetaInfo(filest, meta);
	ASSERT_EQ(dbfile.Open(string("test_open").c_str()), 1);
	dbfile.Close();
}


// TEST(Heap, Create) {
// 	DBFile dbfile = DBFile();
	
// 	OrderMaker sortorder = OrderMaker();
// 	FileMetaInfo meta = {
// 		heap,
// 		100,
// 		sortorder
// 	};
// 	const char *fpath = string("test_create").c_str();
// 	ASSERT_EQ(dbfile.Create(fpath, heap, NULL), 1);
// 	cout<<"63";
// 	dbfile.Close();
// }


int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}