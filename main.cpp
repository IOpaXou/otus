#include "IMovable.h"
#include "Move.h"
#include "MovableObject.h"
#include "Rotate.h"
#include "RotatableObject.h"

#include <climits>
#include <cmath>
#include <exception>
#include <gtest/gtest.h>

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

const auto EPSILON = 1e-9;

std::vector<double> solve(double a, double b, double c)
{
	if (!std::isfinite(a) || !std::isfinite(b) || !std::isfinite(c))
	{
		throw std::invalid_argument("Invalid argument");
	}

	if (std::abs(a) < EPSILON)
	{
		throw std::invalid_argument("Zero A");
	}

	const auto D = b*b - 4*a*c;
	if (D > EPSILON)
	{
		const auto x1 = (-b + std::sqrt(D)) / 2*a;
		const auto x2 = (-b - std::sqrt(D)) / 2*a;
		return {x1, x2};
	}
	else if (D < -EPSILON)
	{
		return {};
	}
	else
	{
		const auto x = -b / (2*a);
		return {x};
	}
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

// Items 3-4. x^2 + 1 = 0
TEST(SolveTestSuite, ABC101)
{
	const auto roots = solve(1, 0, 1);
	ASSERT_EQ(roots.size(), 0);
}

// Items 5-6. x^2 - 1 = 0
TEST(SolveTestSuite, ABC10m1)
{
	const auto roots = solve(1, 0, -1);
	ASSERT_EQ(roots.size(), 2);
	EXPECT_DOUBLE_EQ(roots.at(0), 1);
	EXPECT_DOUBLE_EQ(roots.at(1), -1);
}

// Items 7-8. x^2 + 2*x + 1 = 0
TEST(SolveTestSuite, ABC121)
{
	const auto roots = solve(1, 2, 1);
	ASSERT_EQ(roots.size(), 1);
	EXPECT_DOUBLE_EQ(roots.at(0), -1);
}

// Items 9-10. a = 0
TEST(SolveTestSuite, ZeroA)
{
	EXPECT_THROW(solve(0, 3.1, 2.3), std::invalid_argument);
}

// Items 11-12. x^2 + (2+1e-10)*x + 1+1e-10 = 0
TEST(SolveTestSuite, ABC121em10)
{
	const auto a = 1.0;
	const auto b = 2.0;
	const auto c = 1.0 - (EPSILON/10);
	const auto roots = solve(a, b, c);
	ASSERT_EQ(roots.size(), 1);
	EXPECT_NEAR(roots.at(0), -1, EPSILON);

	const auto D = b*b - 4*a*c;
	EXPECT_LT(D, EPSILON);
	EXPECT_GT(D, 0.0);
}

// Items 13-14. NaN or Infinite coefficients
TEST(SolveTestSuite, NotFiniteABC)
{
	EXPECT_THROW(solve(NAN, 2.2, 10.0), std::invalid_argument);
	EXPECT_THROW(solve(1.0, INFINITY, 10.0), std::invalid_argument);
	EXPECT_THROW(solve(1.0, 2.2, NAN), std::invalid_argument);
}

int main(int argc, char** argv)
{
	testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
