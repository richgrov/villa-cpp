#ifndef VILLA_MATH_VEC2_H_
#define VILLA_MATH_VEC2_H_

namespace villa {

struct Vec2 {
   Vec2() : x(0), y(0) {}
   Vec2(float x_, float y_) : x(x_), y(y_) {}

   float x;
   float y;
};

} // namespace villa

#endif // !VILLA_MATH_VEC2_H_
