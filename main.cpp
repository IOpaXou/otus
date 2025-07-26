#include "IMovable.h"
#include "Move.h"
#include "MovableObject.h"
#include "Rotate.h"
#include "RotatableObject.h"

#include <gtest/gtest.h>
#include <stdexcept>

constexpr Point InitialPos {12, 5};
constexpr Vector InitialVelocity {-7, 3};

class MovableButNotGetLocatable : public IMovable
{
public:
	Point getLocation() override {
		throw std::runtime_error("Can't get location");
	}
	Vector getVelocity() override {return {};}
	void setLocation(const Point&) override {}
};

class MovableButNotGetVelocitable : public IMovable
{
public:
	Point getLocation() override {return {};}
	Vector getVelocity() override {
		throw std::runtime_error("Can't get velocity");
	}
	void setLocation(const Point&) override {}
};

class MovableButNotSetLocatable : public IMovable
{
public:
	Point getLocation() override {return {};}
	Vector getVelocity() override {return {};}
	void setLocation(const Point&) override {
		throw std::runtime_error("Can't set location");
	}
};

int main(int argc, char** argv)
{
	testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}

TEST(MoveTestSuite, MoveObject)
{
	constexpr Point FinalPos {5, 8};

	MovableObject mobj(InitialPos, InitialVelocity);

	Move move(mobj);
	move.exec();

	ASSERT_EQ(mobj.getLocation(), FinalPos);
}

TEST(MoveTestSuite, MoveNoGetLocationObject)
{
	MovableButNotGetLocatable mobj;
	Move move(mobj);

	EXPECT_THROW(move.exec(), std::runtime_error);
}

TEST(MoveTestSuite, MoveNoVelocityObject)
{
	MovableButNotGetVelocitable mobj;
	Move move(mobj);

	EXPECT_THROW(move.exec(), std::runtime_error);
}

TEST(MoveTestSuite, MoveNoSetLocationObject)
{
	MovableButNotSetLocatable mobj;
	Move move(mobj);

	EXPECT_THROW(move.exec(), std::runtime_error);
}

TEST(RotateTestSuite, RotateObject)
{
	RotatableObject robj(10.0, 40.0);
	Rotate rotate(robj);

	rotate.exec();

	ASSERT_EQ(robj.getAngle(), 50.0);

	RotatableObject robj2(10.0, -40.0);
	Rotate rotate2(robj2);

	rotate2.exec();

	ASSERT_EQ(robj2.getAngle(), 330.0);

	RotatableObject robj3(350.0, 300.0);
	Rotate rotate3(robj3);

	rotate3.exec();

	ASSERT_EQ(robj3.getAngle(), 290.0);
}
