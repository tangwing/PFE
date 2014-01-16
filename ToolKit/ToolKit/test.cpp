#include <gtest\gtest.h>
#include "H1Result.h"
#include "CplexResult.h"

TEST(ResultTest, H1IO)
{
	H1Result res(1, 2, 2.333, 4.555,6.777);
	res.ExportToFile("h1.txt");
	H1Result resIn;
	resIn.ImportFromFile("h1.txt");
	EXPECT_EQ(1, resIn.isFeasible);
	EXPECT_EQ(2, resIn.nbMachine);
	EXPECT_EQ(2.333, resIn.value);
	EXPECT_EQ(4.555, resIn.durationWallClock);
	EXPECT_EQ(6.777, resIn.durationCpuClock);
}

TEST(ResultTest, CplexIO)
{
	CplexResult res(1,0,1,1, 2, 10, 102,2.333, 4.555,6.777);
	res.ExportToFile("h2.txt");
	CplexResult resIn;
	resIn.ImportFromFile("h2.txt");
	EXPECT_EQ(1, resIn.isFeasible);
	EXPECT_EQ(0, resIn.isOptimal);
	EXPECT_EQ(2, resIn.nbMachine);
	EXPECT_EQ(10, resIn.nbNode);
	EXPECT_EQ(102, resIn.statusCode);
	EXPECT_EQ(2.333, resIn.value);
	EXPECT_EQ(4.555, resIn.durationWallClock);
	EXPECT_EQ(6.777, resIn.durationCpuClock);
}