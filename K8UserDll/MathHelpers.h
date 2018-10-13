#pragma once

//-------------------------------------------------------------------------------------
// ***** Size

// Size class represents 2D size with Width, Height components.
// Used to describe distentions of render targets, etc.

template <class T>
class Size {
public:
	T w, h;

	Size() : w(0), h(0) {}
	Size(T w_, T h_) : w(w_), h(h_) {}
	explicit Size(T s) : w(s), h(s) {}
	explicit Size(const Size<typename Math<T>::OtherFloatType>& src) : w((T)src.w), h((T)src.h) {}

	// C-interop support.
	typedef typename CompatibleTypes<Size<T>>::Type CompatibleType;

	Size(const CompatibleType& s) : w(s.w), h(s.h) {}

	operator const CompatibleType&() const {
		OVR_MATH_STATIC_ASSERT(sizeof(Size<T>) == sizeof(CompatibleType), "sizeof(Size<T>) failure");
		return reinterpret_cast<const CompatibleType&>(*this);
	}

	bool operator==(const Size& b) const {
		return w == b.w && h == b.h;
	}
	bool operator!=(const Size& b) const {
		return w != b.w || h != b.h;
	}

	Size operator+(const Size& b) const {
		return Size(w + b.w, h + b.h);
	}
	Size& operator+=(const Size& b) {
		w += b.w;
		h += b.h;
		return *this;
	}
	Size operator-(const Size& b) const {
		return Size(w - b.w, h - b.h);
	}
	Size& operator-=(const Size& b) {
		w -= b.w;
		h -= b.h;
		return *this;
	}
	Size operator-() const {
		return Size(-w, -h);
	}
	Size operator*(const Size& b) const {
		return Size(w * b.w, h * b.h);
	}
	Size& operator*=(const Size& b) {
		w *= b.w;
		h *= b.h;
		return *this;
	}
	Size operator/(const Size& b) const {
		return Size(w / b.w, h / b.h);
	}
	Size& operator/=(const Size& b) {
		w /= b.w;
		h /= b.h;
		return *this;
	}

	// Scalar multiplication/division scales both components.
	Size operator*(T s) const {
		return Size(w * s, h * s);
	}
	Size& operator*=(T s) {
		w *= s;
		h *= s;
		return *this;
	}
	Size operator/(T s) const {
		return Size(w / s, h / s);
	}
	Size& operator/=(T s) {
		w /= s;
		h /= s;
		return *this;
	}

	static Size Min(const Size& a, const Size& b) {
		return Size((a.w < b.w) ? a.w : b.w, (a.h < b.h) ? a.h : b.h);
	}
	static Size Max(const Size& a, const Size& b) {
		return Size((a.w > b.w) ? a.w : b.w, (a.h > b.h) ? a.h : b.h);
	}

	T Area() const {
		return w * h;
	}

	inline Vector2<T> ToVector() const {
		return Vector2<T>(w, h);
	}
};

typedef Size<int> Sizei;
typedef Size<unsigned> Sizeu;
typedef Size<float> Sizef;
typedef Size<double> Sized;






//-------------------------------------------------------------------------------------
// ***** Matrix4
//
// Matrix4 is a 4x4 matrix used for 3d transformations and projections.
// Translation stored in the last column.
// The matrix is stored in row-major order in memory, meaning that values
// of the first row are stored before the next one.
//
// The arrangement of the matrix is chosen to be in Right-Handed
// coordinate system and counterclockwise rotations when looking down
// the axis
//
// Transformation Order:
//   - Transformations are applied from right to left, so the expression
//     M1 * M2 * M3 * V means that the vector V is transformed by M3 first,
//     followed by M2 and M1.
//
// Coordinate system: Right Handed
//
// Rotations: Counterclockwise when looking down the axis. All angles are in radians.
//
//  | sx   01   02   tx |    // First column  (sx, 10, 20): Axis X basis vector.
//  | 10   sy   12   ty |    // Second column (01, sy, 21): Axis Y basis vector.
//  | 20   21   sz   tz |    // Third columnt (02, 12, sz): Axis Z basis vector.
//  | 30   31   32   33 |
//
//  The basis vectors are first three columns.

template <class T>
class Matrix4 {
 public:
  typedef T ElementType;
  static const size_t Dimension = 4;

  T M[4][4];

  enum NoInitType { NoInit };

  // Construct with no memory initialization.
  Matrix4(NoInitType) {}

  // By default, we construct identity matrix.
  Matrix4() {
    M[0][0] = M[1][1] = M[2][2] = M[3][3] = T(1);
    M[0][1] = M[1][0] = M[2][3] = M[3][1] = T(0);
    M[0][2] = M[1][2] = M[2][0] = M[3][2] = T(0);
    M[0][3] = M[1][3] = M[2][1] = M[3][0] = T(0);
  }

  Matrix4(
      T m11,
      T m12,
      T m13,
      T m14,
      T m21,
      T m22,
      T m23,
      T m24,
      T m31,
      T m32,
      T m33,
      T m34,
      T m41,
      T m42,
      T m43,
      T m44) {
    M[0][0] = m11;
    M[0][1] = m12;
    M[0][2] = m13;
    M[0][3] = m14;
    M[1][0] = m21;
    M[1][1] = m22;
    M[1][2] = m23;
    M[1][3] = m24;
    M[2][0] = m31;
    M[2][1] = m32;
    M[2][2] = m33;
    M[2][3] = m34;
    M[3][0] = m41;
    M[3][1] = m42;
    M[3][2] = m43;
    M[3][3] = m44;
  }

  Matrix4(T m11, T m12, T m13, T m21, T m22, T m23, T m31, T m32, T m33) {
    M[0][0] = m11;
    M[0][1] = m12;
    M[0][2] = m13;
    M[0][3] = T(0);
    M[1][0] = m21;
    M[1][1] = m22;
    M[1][2] = m23;
    M[1][3] = T(0);
    M[2][0] = m31;
    M[2][1] = m32;
    M[2][2] = m33;
    M[2][3] = T(0);
    M[3][0] = T(0);
    M[3][1] = T(0);
    M[3][2] = T(0);
    M[3][3] = T(1);
  }

  explicit Matrix4(const Matrix3<T>& m) {
    M[0][0] = m.M[0][0];
    M[0][1] = m.M[0][1];
    M[0][2] = m.M[0][2];
    M[0][3] = T(0);
    M[1][0] = m.M[1][0];
    M[1][1] = m.M[1][1];
    M[1][2] = m.M[1][2];
    M[1][3] = T(0);
    M[2][0] = m.M[2][0];
    M[2][1] = m.M[2][1];
    M[2][2] = m.M[2][2];
    M[2][3] = T(0);
    M[3][0] = T(0);
    M[3][1] = T(0);
    M[3][2] = T(0);
    M[3][3] = T(1);
  }


  explicit Matrix4(const glm::quat<T>& q) {
	  T ww = q.w * q.w;
	  T xx = q.x * q.x;
	  T yy = q.y * q.y;
	  T zz = q.z * q.z;

	  M[0][0] = ww + xx - yy - zz;
	  M[0][1] = 2 * (q.x * q.y - q.w * q.z);
	  M[0][2] = 2 * (q.x * q.z + q.w * q.y);
	  M[0][3] = T(0);
	  M[1][0] = 2 * (q.x * q.y + q.w * q.z);
	  M[1][1] = ww - xx + yy - zz;
	  M[1][2] = 2 * (q.y * q.z - q.w * q.x);
	  M[1][3] = T(0);
	  M[2][0] = 2 * (q.x * q.z - q.w * q.y);
	  M[2][1] = 2 * (q.y * q.z + q.w * q.x);
	  M[2][2] = ww - xx - yy + zz;
	  M[2][3] = T(0);
	  M[3][0] = T(0);
	  M[3][1] = T(0);
	  M[3][2] = T(0);
	  M[3][3] = T(1);
  }


  /*
  explicit Matrix4(const Quat<T>& q) {
    OVR_MATH_ASSERT(q.IsNormalized()); // If this fires, caller has a quat math bug
    T ww = q.w * q.w;
    T xx = q.x * q.x;
    T yy = q.y * q.y;
    T zz = q.z * q.z;

    M[0][0] = ww + xx - yy - zz;
    M[0][1] = 2 * (q.x * q.y - q.w * q.z);
    M[0][2] = 2 * (q.x * q.z + q.w * q.y);
    M[0][3] = T(0);
    M[1][0] = 2 * (q.x * q.y + q.w * q.z);
    M[1][1] = ww - xx + yy - zz;
    M[1][2] = 2 * (q.y * q.z - q.w * q.x);
    M[1][3] = T(0);
    M[2][0] = 2 * (q.x * q.z - q.w * q.y);
    M[2][1] = 2 * (q.y * q.z + q.w * q.x);
    M[2][2] = ww - xx - yy + zz;
    M[2][3] = T(0);
    M[3][0] = T(0);
    M[3][1] = T(0);
    M[3][2] = T(0);
    M[3][3] = T(1);
  }


  explicit Matrix4(const Pose<T>& p) {
    Matrix4 result(p.Rotation);
    result.SetTranslation(p.Translation);
    *this = result;
  }
  */

  // C-interop support
  explicit Matrix4(const Matrix4<typename Math<T>::OtherFloatType>& src) {
    for (int i = 0; i < 4; i++)
      for (int j = 0; j < 4; j++)
        M[i][j] = (T)src.M[i][j];
  }

  // C-interop support.
  Matrix4(const typename CompatibleTypes<Matrix4<T>>::Type& s) {
    OVR_MATH_STATIC_ASSERT(sizeof(s) == sizeof(Matrix4), "sizeof(s) == sizeof(Matrix4)");
    memcpy(M, s.M, sizeof(M));
  }

  operator typename CompatibleTypes<Matrix4<T>>::Type() const {
    typename CompatibleTypes<Matrix4<T>>::Type result;
    OVR_MATH_STATIC_ASSERT(sizeof(result) == sizeof(Matrix4), "sizeof(result) == sizeof(Matrix4)");
    memcpy(result.M, M, sizeof(M));
    return result;
  }

  void ToString(char* dest, size_t destsize) const {
    size_t pos = 0;
    for (int r = 0; r < 4; r++) {
      for (int c = 0; c < 4; c++) {
        pos += OVRMath_sprintf(dest + pos, destsize - pos, "%g ", M[r][c]);
      }
    }
  }

  static Matrix4 FromString(const char* src) {
    Matrix4 result;
    if (src) {
      for (int r = 0; r < 4; r++) {
        for (int c = 0; c < 4; c++) {
          result.M[r][c] = (T)atof(src);
          while (*src && *src != ' ') {
            src++;
          }
          while (*src && *src == ' ') {
            src++;
          }
        }
      }
    }
    return result;
  }

  static Matrix4 Identity() {
    return Matrix4();
  }

  void SetIdentity() {
    M[0][0] = M[1][1] = M[2][2] = M[3][3] = T(1);
    M[0][1] = M[1][0] = M[2][3] = M[3][1] = T(0);
    M[0][2] = M[1][2] = M[2][0] = M[3][2] = T(0);
    M[0][3] = M[1][3] = M[2][1] = M[3][0] = T(0);
  }

  void SetXBasis(const Vector3<T>& v) {
    M[0][0] = v.x;
    M[1][0] = v.y;
    M[2][0] = v.z;
  }
  Vector3<T> GetXBasis() const {
    return Vector3<T>(M[0][0], M[1][0], M[2][0]);
  }

  void SetYBasis(const Vector3<T>& v) {
    M[0][1] = v.x;
    M[1][1] = v.y;
    M[2][1] = v.z;
  }
  Vector3<T> GetYBasis() const {
    return Vector3<T>(M[0][1], M[1][1], M[2][1]);
  }

  void SetZBasis(const Vector3<T>& v) {
    M[0][2] = v.x;
    M[1][2] = v.y;
    M[2][2] = v.z;
  }
  Vector3<T> GetZBasis() const {
    return Vector3<T>(M[0][2], M[1][2], M[2][2]);
  }

  bool operator==(const Matrix4& b) const {
    bool isEqual = true;
    for (int i = 0; i < 4; i++)
      for (int j = 0; j < 4; j++)
        isEqual &= (M[i][j] == b.M[i][j]);

    return isEqual;
  }

  Matrix4 operator+(const Matrix4& b) const {
    Matrix4 result(*this);
    result += b;
    return result;
  }

  Matrix4& operator+=(const Matrix4& b) {
    for (int i = 0; i < 4; i++)
      for (int j = 0; j < 4; j++)
        M[i][j] += b.M[i][j];
    return *this;
  }

  Matrix4 operator-(const Matrix4& b) const {
    Matrix4 result(*this);
    result -= b;
    return result;
  }

  Matrix4& operator-=(const Matrix4& b) {
    for (int i = 0; i < 4; i++)
      for (int j = 0; j < 4; j++)
        M[i][j] -= b.M[i][j];
    return *this;
  }

  // Multiplies two matrices into destination with minimum copying.
  static Matrix4& Multiply(Matrix4* d, const Matrix4& a, const Matrix4& b) {
    OVR_MATH_ASSERT((d != &a) && (d != &b));
    int i = 0;
    do {
      d->M[i][0] = a.M[i][0] * b.M[0][0] + a.M[i][1] * b.M[1][0] + a.M[i][2] * b.M[2][0] +
          a.M[i][3] * b.M[3][0];
      d->M[i][1] = a.M[i][0] * b.M[0][1] + a.M[i][1] * b.M[1][1] + a.M[i][2] * b.M[2][1] +
          a.M[i][3] * b.M[3][1];
      d->M[i][2] = a.M[i][0] * b.M[0][2] + a.M[i][1] * b.M[1][2] + a.M[i][2] * b.M[2][2] +
          a.M[i][3] * b.M[3][2];
      d->M[i][3] = a.M[i][0] * b.M[0][3] + a.M[i][1] * b.M[1][3] + a.M[i][2] * b.M[2][3] +
          a.M[i][3] * b.M[3][3];
    } while ((++i) < 4);

    return *d;
  }

  Matrix4 operator*(const Matrix4& b) const {
    Matrix4 result(Matrix4::NoInit);
    Multiply(&result, *this, b);
    return result;
  }

  Matrix4& operator*=(const Matrix4& b) {
    return Multiply(this, Matrix4(*this), b);
  }

  Matrix4 operator*(T s) const {
    Matrix4 result(*this);
    result *= s;
    return result;
  }

  Matrix4& operator*=(T s) {
    for (int i = 0; i < 4; i++)
      for (int j = 0; j < 4; j++)
        M[i][j] *= s;
    return *this;
  }

  Matrix4 operator/(T s) const {
    Matrix4 result(*this);
    result /= s;
    return result;
  }

  Matrix4& operator/=(T s) {
    for (int i = 0; i < 4; i++)
      for (int j = 0; j < 4; j++)
        M[i][j] /= s;
    return *this;
  }

  T operator()(int i, int j) const {
    return M[i][j];
  }
  T& operator()(int i, int j) {
    return M[i][j];
  }

  Vector4<T> operator*(const Vector4<T>& b) const {
    return Transform(b);
  }

  Vector3<T> Transform(const Vector3<T>& v) const {
    const T rcpW = T(1) / (M[3][0] * v.x + M[3][1] * v.y + M[3][2] * v.z + M[3][3]);
    return Vector3<T>(
        (M[0][0] * v.x + M[0][1] * v.y + M[0][2] * v.z + M[0][3]) * rcpW,
        (M[1][0] * v.x + M[1][1] * v.y + M[1][2] * v.z + M[1][3]) * rcpW,
        (M[2][0] * v.x + M[2][1] * v.y + M[2][2] * v.z + M[2][3]) * rcpW);
  }

  Vector4<T> Transform(const Vector4<T>& v) const {
    return Vector4<T>(
        M[0][0] * v.x + M[0][1] * v.y + M[0][2] * v.z + M[0][3] * v.w,
        M[1][0] * v.x + M[1][1] * v.y + M[1][2] * v.z + M[1][3] * v.w,
        M[2][0] * v.x + M[2][1] * v.y + M[2][2] * v.z + M[2][3] * v.w,
        M[3][0] * v.x + M[3][1] * v.y + M[3][2] * v.z + M[3][3] * v.w);
  }

  Matrix4 Transposed() const {
    return Matrix4(
        M[0][0],
        M[1][0],
        M[2][0],
        M[3][0],
        M[0][1],
        M[1][1],
        M[2][1],
        M[3][1],
        M[0][2],
        M[1][2],
        M[2][2],
        M[3][2],
        M[0][3],
        M[1][3],
        M[2][3],
        M[3][3]);
  }

  void Transpose() {
    *this = Transposed();
  }

  T SubDet(const size_t* rows, const size_t* cols) const {
    return M[rows[0]][cols[0]] *
        (M[rows[1]][cols[1]] * M[rows[2]][cols[2]] - M[rows[1]][cols[2]] * M[rows[2]][cols[1]]) -
        M[rows[0]][cols[1]] *
        (M[rows[1]][cols[0]] * M[rows[2]][cols[2]] - M[rows[1]][cols[2]] * M[rows[2]][cols[0]]) +
        M[rows[0]][cols[2]] *
        (M[rows[1]][cols[0]] * M[rows[2]][cols[1]] - M[rows[1]][cols[1]] * M[rows[2]][cols[0]]);
  }

  T Cofactor(size_t I, size_t J) const {
    const size_t indices[4][3] = {{1, 2, 3}, {0, 2, 3}, {0, 1, 3}, {0, 1, 2}};
    return ((I + J) & 1) ? -SubDet(indices[I], indices[J]) : SubDet(indices[I], indices[J]);
  }

  T Determinant() const {
    return M[0][0] * Cofactor(0, 0) + M[0][1] * Cofactor(0, 1) + M[0][2] * Cofactor(0, 2) +
        M[0][3] * Cofactor(0, 3);
  }

  Matrix4 Adjugated() const {
    return Matrix4(
        Cofactor(0, 0),
        Cofactor(1, 0),
        Cofactor(2, 0),
        Cofactor(3, 0),
        Cofactor(0, 1),
        Cofactor(1, 1),
        Cofactor(2, 1),
        Cofactor(3, 1),
        Cofactor(0, 2),
        Cofactor(1, 2),
        Cofactor(2, 2),
        Cofactor(3, 2),
        Cofactor(0, 3),
        Cofactor(1, 3),
        Cofactor(2, 3),
        Cofactor(3, 3));
  }

  Matrix4 Inverted() const {
    T det = Determinant();
    OVR_MATH_ASSERT(det != 0);
    return Adjugated() * (T(1) / det);
  }

  void Invert() {
    *this = Inverted();
  }

  // This is more efficient than general inverse, but ONLY works
  // correctly if it is a homogeneous transform matrix (rot + trans)
  Matrix4 InvertedHomogeneousTransform() const {
    // Make the inverse rotation matrix
    Matrix4 rinv = this->Transposed();
    rinv.M[3][0] = rinv.M[3][1] = rinv.M[3][2] = T(0);
    // Make the inverse translation matrix
    Vector3<T> tvinv(-M[0][3], -M[1][3], -M[2][3]);
    Matrix4 tinv = Matrix4::Translation(tvinv);
    return rinv * tinv; // "untranslate", then "unrotate"
  }

  // This is more efficient than general inverse, but ONLY works
  // correctly if it is a homogeneous transform matrix (rot + trans)
  void InvertHomogeneousTransform() {
    *this = InvertedHomogeneousTransform();
  }

  // Matrix to Euler Angles conversion
  // a,b,c, are the YawPitchRoll angles to be returned
  // rotation a around axis A1
  // is followed by rotation b around axis A2
  // is followed by rotation c around axis A3
  // rotations are CCW or CW (D) in LH or RH coordinate system (S)
  template <Axis A1, Axis A2, Axis A3, RotateDirection D, HandedSystem S>
  void ToEulerAngles(T* a, T* b, T* c) const {
    OVR_MATH_STATIC_ASSERT(
        (A1 != A2) && (A2 != A3) && (A1 != A3), "(A1 != A2) && (A2 != A3) && (A1 != A3)");

    T psign = T(-1);
    if (((A1 + 1) % 3 == A2) && ((A2 + 1) % 3 == A3)) // Determine whether even permutation
      psign = T(1);

    T pm = psign * M[A1][A3];
    T singularityRadius = Math<T>::SingularityRadius();
    if (pm < T(-1) + singularityRadius) { // South pole singularity
      *a = T(0);
      *b = -S * D * ((T)MATH_DOUBLE_PIOVER2);
      *c = S * D * atan2(psign * M[A2][A1], M[A2][A2]);
    } else if (pm > T(1) - singularityRadius) { // North pole singularity
      *a = T(0);
      *b = S * D * ((T)MATH_DOUBLE_PIOVER2);
      *c = S * D * atan2(psign * M[A2][A1], M[A2][A2]);
    } else { // Normal case (nonsingular)
      *a = S * D * atan2(-psign * M[A2][A3], M[A3][A3]);
      *b = S * D * asin(pm);
      *c = S * D * atan2(-psign * M[A1][A2], M[A1][A1]);
    }
  }

  // Matrix to Euler Angles conversion
  // a,b,c, are the YawPitchRoll angles to be returned
  // rotation a around axis A1
  // is followed by rotation b around axis A2
  // is followed by rotation c around axis A1
  // rotations are CCW or CW (D) in LH or RH coordinate system (S)
  template <Axis A1, Axis A2, RotateDirection D, HandedSystem S>
  void ToEulerAnglesABA(T* a, T* b, T* c) const {
    OVR_MATH_STATIC_ASSERT(A1 != A2, "A1 != A2");

    // Determine the axis that was not supplied
    int m = 3 - A1 - A2;

    T psign = T(-1);
    if ((A1 + 1) % 3 == A2) // Determine whether even permutation
      psign = T(1);

    T c2 = M[A1][A1];
    T singularityRadius = Math<T>::SingularityRadius();
    if (c2 < T(-1) + singularityRadius) { // South pole singularity
      *a = T(0);
      *b = S * D * ((T)MATH_DOUBLE_PI);
      *c = S * D * atan2(-psign * M[A2][m], M[A2][A2]);
    } else if (c2 > T(1) - singularityRadius) { // North pole singularity
      *a = T(0);
      *b = T(0);
      *c = S * D * atan2(-psign * M[A2][m], M[A2][A2]);
    } else { // Normal case (nonsingular)
      *a = S * D * atan2(M[A2][A1], -psign * M[m][A1]);
      *b = S * D * acos(c2);
      *c = S * D * atan2(M[A1][A2], psign * M[A1][m]);
    }
  }

  // Creates a matrix that converts the vertices from one coordinate system
  // to another.
  static Matrix4 AxisConversion(const WorldAxes& to, const WorldAxes& from) {
    // Holds axis values from the 'to' structure
    int toArray[3] = {to.XAxis, to.YAxis, to.ZAxis};

    // The inverse of the toArray
    int inv[4];
    inv[0] = inv[abs(to.XAxis)] = 0;
    inv[abs(to.YAxis)] = 1;
    inv[abs(to.ZAxis)] = 2;

    Matrix4 m(0, 0, 0, 0, 0, 0, 0, 0, 0);

    // Only three values in the matrix need to be changed to 1 or -1.
    m.M[inv[abs(from.XAxis)]][0] = T(from.XAxis / toArray[inv[abs(from.XAxis)]]);
    m.M[inv[abs(from.YAxis)]][1] = T(from.YAxis / toArray[inv[abs(from.YAxis)]]);
    m.M[inv[abs(from.ZAxis)]][2] = T(from.ZAxis / toArray[inv[abs(from.ZAxis)]]);
    return m;
  }

  // Creates a matrix for translation by vector
  static Matrix4 Translation(const Vector3<T>& v) {
    Matrix4 t;
    t.M[0][3] = v.x;
    t.M[1][3] = v.y;
    t.M[2][3] = v.z;
    return t;
  }

  // Creates a matrix for translation by vector
  static Matrix4 Translation(T x, T y, T z = T(0)) {
    Matrix4 t;
    t.M[0][3] = x;
    t.M[1][3] = y;
    t.M[2][3] = z;
    return t;
  }

  // Sets the translation part
  void SetTranslation(const Vector3<T>& v) {
    M[0][3] = v.x;
    M[1][3] = v.y;
    M[2][3] = v.z;
  }

  Vector3<T> GetTranslation() const {
    return Vector3<T>(M[0][3], M[1][3], M[2][3]);
  }

  // Creates a matrix for scaling by vector
  static Matrix4 Scaling(const Vector3<T>& v) {
    Matrix4 t;
    t.M[0][0] = v.x;
    t.M[1][1] = v.y;
    t.M[2][2] = v.z;
    return t;
  }

  // Creates a matrix for scaling by vector
  static Matrix4 Scaling(T x, T y, T z) {
    Matrix4 t;
    t.M[0][0] = x;
    t.M[1][1] = y;
    t.M[2][2] = z;
    return t;
  }

  // Creates a matrix for scaling by constant
  static Matrix4 Scaling(T s) {
    Matrix4 t;
    t.M[0][0] = s;
    t.M[1][1] = s;
    t.M[2][2] = s;
    return t;
  }

  // Simple L1 distance in R^12
  T Distance(const Matrix4& m2) const {
    T d = fabs(M[0][0] - m2.M[0][0]) + fabs(M[0][1] - m2.M[0][1]);
    d += fabs(M[0][2] - m2.M[0][2]) + fabs(M[0][3] - m2.M[0][3]);
    d += fabs(M[1][0] - m2.M[1][0]) + fabs(M[1][1] - m2.M[1][1]);
    d += fabs(M[1][2] - m2.M[1][2]) + fabs(M[1][3] - m2.M[1][3]);
    d += fabs(M[2][0] - m2.M[2][0]) + fabs(M[2][1] - m2.M[2][1]);
    d += fabs(M[2][2] - m2.M[2][2]) + fabs(M[2][3] - m2.M[2][3]);
    d += fabs(M[3][0] - m2.M[3][0]) + fabs(M[3][1] - m2.M[3][1]);
    d += fabs(M[3][2] - m2.M[3][2]) + fabs(M[3][3] - m2.M[3][3]);
    return d;
  }

  // Creates a rotation matrix rotating around the X axis by 'angle' radians.
  // Just for quick testing.  Not for final API.  Need to remove case.
  static Matrix4 RotationAxis(Axis A, T angle, RotateDirection d, HandedSystem s) {
    T sina = s * d * sin(angle);
    T cosa = cos(angle);

    switch (A) {
      case Axis_X:
        return Matrix4(1, 0, 0, 0, cosa, -sina, 0, sina, cosa);
      case Axis_Y:
        return Matrix4(cosa, 0, sina, 0, 1, 0, -sina, 0, cosa);
      case Axis_Z:
        return Matrix4(cosa, -sina, 0, sina, cosa, 0, 0, 0, 1);
      default:
        return Matrix4();
    }
  }

  // Creates a rotation matrix rotating around the X axis by 'angle' radians.
  // Rotation direction is depends on the coordinate system:
  // RHS (Oculus default): Positive angle values rotate Counter-clockwise (CCW),
  //                        while looking in the negative axis direction. This is the
  //                        same as looking down from positive axis values towards origin.
  // LHS: Positive angle values rotate clock-wise (CW), while looking in the
  //       negative axis direction.
  static Matrix4 RotationX(T angle) {
    T sina = sin(angle);
    T cosa = cos(angle);
    return Matrix4(1, 0, 0, 0, cosa, -sina, 0, sina, cosa);
  }

  // Creates a rotation matrix rotating around the Y axis by 'angle' radians.
  // Rotation direction is depends on the coordinate system:
  //  RHS (Oculus default): Positive angle values rotate Counter-clockwise (CCW),
  //                        while looking in the negative axis direction. This is the
  //                        same as looking down from positive axis values towards origin.
  //  LHS: Positive angle values rotate clock-wise (CW), while looking in the
  //       negative axis direction.
  static Matrix4 RotationY(T angle) {
    T sina = (T)sin(angle);
    T cosa = (T)cos(angle);
    return Matrix4(cosa, 0, sina, 0, 1, 0, -sina, 0, cosa);
  }

  // Creates a rotation matrix rotating around the Z axis by 'angle' radians.
  // Rotation direction is depends on the coordinate system:
  //  RHS (Oculus default): Positive angle values rotate Counter-clockwise (CCW),
  //                        while looking in the negative axis direction. This is the
  //                        same as looking down from positive axis values towards origin.
  //  LHS: Positive angle values rotate clock-wise (CW), while looking in the
  //       negative axis direction.
  static Matrix4 RotationZ(T angle) {
    T sina = sin(angle);
    T cosa = cos(angle);
    return Matrix4(cosa, -sina, 0, sina, cosa, 0, 0, 0, 1);
  }

  // LookAtRH creates a View transformation matrix for right-handed coordinate system.
  // The resulting matrix points camera from 'eye' towards 'at' direction, with 'up'
  // specifying the up vector. The resulting matrix should be used with PerspectiveRH
  // projection.
  static Matrix4 LookAtRH(const Vector3<T>& eye, const Vector3<T>& at, const Vector3<T>& up) {
    Vector3<T> z = (eye - at).Normalized(); // Forward
    Vector3<T> x = up.Cross(z).Normalized(); // Right
    Vector3<T> y = z.Cross(x);

    Matrix4 m(
        x.x,
        x.y,
        x.z,
        -(x.Dot(eye)),
        y.x,
        y.y,
        y.z,
        -(y.Dot(eye)),
        z.x,
        z.y,
        z.z,
        -(z.Dot(eye)),
        0,
        0,
        0,
        1);
    return m;
  }

  // LookAtLH creates a View transformation matrix for left-handed coordinate system.
  // The resulting matrix points camera from 'eye' towards 'at' direction, with 'up'
  // specifying the up vector.
  static Matrix4 LookAtLH(const Vector3<T>& eye, const Vector3<T>& at, const Vector3<T>& up) {
    Vector3<T> z = (at - eye).Normalized(); // Forward
    Vector3<T> x = up.Cross(z).Normalized(); // Right
    Vector3<T> y = z.Cross(x);

    Matrix4 m(
        x.x,
        x.y,
        x.z,
        -(x.Dot(eye)),
        y.x,
        y.y,
        y.z,
        -(y.Dot(eye)),
        z.x,
        z.y,
        z.z,
        -(z.Dot(eye)),
        0,
        0,
        0,
        1);
    return m;
  }

  // PerspectiveRH creates a right-handed perspective projection matrix that can be
  // used with the Oculus sample renderer.
  //  yfov   - Specifies vertical field of view in radians.
  //  aspect - Screen aspect ration, which is usually width/height for square pixels.
  //           Note that xfov = yfov * aspect.
  //  znear  - Absolute value of near Z clipping clipping range.
  //  zfar   - Absolute value of far  Z clipping clipping range (larger then near).
  // Even though RHS usually looks in the direction of negative Z, positive values
  // are expected for znear and zfar.
  static Matrix4 PerspectiveRH(T yfov, T aspect, T znear, T zfar) {
    Matrix4 m;
    T tanHalfFov = (T)tan(yfov * T(0.5));

    m.M[0][0] = T(1) / (aspect * tanHalfFov);
    m.M[1][1] = T(1) / tanHalfFov;
    m.M[2][2] = zfar / (znear - zfar);
    m.M[3][2] = T(-1);
    m.M[2][3] = (zfar * znear) / (znear - zfar);
    m.M[3][3] = T(0);

    // Note: Post-projection matrix result assumes Left-Handed coordinate system,
    //       with Y up, X right and Z forward. This supports positive z-buffer values.
    // This is the case even for RHS coordinate input.
    return m;
  }

  // PerspectiveLH creates a left-handed perspective projection matrix that can be
  // used with the Oculus sample renderer.
  //  yfov   - Specifies vertical field of view in radians.
  //  aspect - Screen aspect ration, which is usually width/height for square pixels.
  //           Note that xfov = yfov * aspect.
  //  znear  - Absolute value of near Z clipping clipping range.
  //  zfar   - Absolute value of far  Z clipping clipping range (larger then near).
  static Matrix4 PerspectiveLH(T yfov, T aspect, T znear, T zfar) {
    Matrix4 m;
    T tanHalfFov = (T)tan(yfov * T(0.5));

    m.M[0][0] = T(1) / (aspect * tanHalfFov);
    m.M[1][1] = T(1) / tanHalfFov;
    // m.M[2][2] = zfar / (znear - zfar);
    m.M[2][2] = zfar / (zfar - znear);
    m.M[3][2] = T(-1);
    m.M[2][3] = (zfar * znear) / (znear - zfar);
    m.M[3][3] = T(0);

    // Note: Post-projection matrix result assumes Left-Handed coordinate system,
    //       with Y up, X right and Z forward. This supports positive z-buffer values.
    // This is the case even for RHS coordinate input.
    return m;
  }

  static Matrix4 Ortho2D(T w, T h) {
    Matrix4 m;
    m.M[0][0] = T(2.0) / w;
    m.M[1][1] = T(-2.0) / h;
    m.M[0][3] = T(-1.0);
    m.M[1][3] = T(1.0);
    m.M[2][2] = T(0);
    return m;
  }
};

typedef Matrix4<float> Matrix4f;
typedef Matrix4<double> Matrix4d;





//-------------------------------------------------------------------------------------
// ***** Vector3<> - 3D vector of {x, y, z}

//
// Vector3f (Vector3d) represents a 3-dimensional vector or point in space,
// consisting of coordinates x, y and z.

template <class T>
class Vector3 {
public:
	typedef T ElementType;
	static const size_t ElementCount = 3;

	T x, y, z;

	// FIXME: default initialization of a vector class can be very expensive in a full-blown
	// application.  A few hundred thousand vector constructions is not unlikely and can add
	// up to milliseconds of time on processors like the PS3 PPU.
	Vector3() : x(0), y(0), z(0) {}
	Vector3(T x_, T y_, T z_ = 0) : x(x_), y(y_), z(z_) {}
	explicit Vector3(T s) : x(s), y(s), z(s) {}
	explicit Vector3(const Vector3<typename Math<T>::OtherFloatType>& src)
		: x((T)src.x), y((T)src.y), z((T)src.z) {}

	static Vector3 Zero() {
		return Vector3(0, 0, 0);
	}

	// C-interop support.
	typedef typename CompatibleTypes<Vector3<T>>::Type CompatibleType;

	Vector3(const CompatibleType& s) : x(s.x), y(s.y), z(s.z) {}

	operator const CompatibleType&() const {
		OVR_MATH_STATIC_ASSERT(
			sizeof(Vector3<T>) == sizeof(CompatibleType), "sizeof(Vector3<T>) failure");
		return reinterpret_cast<const CompatibleType&>(*this);
	}

	bool operator==(const Vector3& b) const {
		return x == b.x && y == b.y && z == b.z;
	}
	bool operator!=(const Vector3& b) const {
		return x != b.x || y != b.y || z != b.z;
	}

	Vector3 operator+(const Vector3& b) const {
		return Vector3(x + b.x, y + b.y, z + b.z);
	}
	Vector3& operator+=(const Vector3& b) {
		x += b.x;
		y += b.y;
		z += b.z;
		return *this;
	}
	Vector3 operator-(const Vector3& b) const {
		return Vector3(x - b.x, y - b.y, z - b.z);
	}
	Vector3& operator-=(const Vector3& b) {
		x -= b.x;
		y -= b.y;
		z -= b.z;
		return *this;
	}
	Vector3 operator-() const {
		return Vector3(-x, -y, -z);
	}

	// Scalar multiplication/division scales vector.
	Vector3 operator*(T s) const {
		return Vector3(x * s, y * s, z * s);
	}
	Vector3& operator*=(T s) {
		x *= s;
		y *= s;
		z *= s;
		return *this;
	}

	Vector3 operator/(T s) const {
		T rcp = T(1) / s;
		return Vector3(x * rcp, y * rcp, z * rcp);
	}
	Vector3& operator/=(T s) {
		T rcp = T(1) / s;
		x *= rcp;
		y *= rcp;
		z *= rcp;
		return *this;
	}

	static Vector3 Min(const Vector3& a, const Vector3& b) {
		return Vector3((a.x < b.x) ? a.x : b.x, (a.y < b.y) ? a.y : b.y, (a.z < b.z) ? a.z : b.z);
	}
	static Vector3 Max(const Vector3& a, const Vector3& b) {
		return Vector3((a.x > b.x) ? a.x : b.x, (a.y > b.y) ? a.y : b.y, (a.z > b.z) ? a.z : b.z);
	}

	Vector3 Clamped(T maxMag) const {
		T magSquared = LengthSq();
		if (magSquared <= Sqr(maxMag))
			return *this;
		else
			return *this * (maxMag / sqrt(magSquared));
	}

	// Compare two vectors for equality with tolerance. Returns true if vectors match within
	// tolerance.
	bool IsEqual(const Vector3& b, T tolerance = Math<T>::Tolerance()) const {
		return (fabs(b.x - x) <= tolerance) && (fabs(b.y - y) <= tolerance) &&
			(fabs(b.z - z) <= tolerance);
	}
	bool Compare(const Vector3& b, T tolerance = Math<T>::Tolerance()) const {
		return IsEqual(b, tolerance);
	}

	T& operator[](int idx) {
		OVR_MATH_ASSERT(0 <= idx && idx < 3);
		return *(&x + idx);
	}

	const T& operator[](int idx) const {
		OVR_MATH_ASSERT(0 <= idx && idx < 3);
		return *(&x + idx);
	}

	// Entrywise product of two vectors
	Vector3 EntrywiseMultiply(const Vector3& b) const {
		return Vector3(x * b.x, y * b.y, z * b.z);
	}

	// Multiply and divide operators do entry-wise math
	Vector3 operator*(const Vector3& b) const {
		return Vector3(x * b.x, y * b.y, z * b.z);
	}

	Vector3 operator/(const Vector3& b) const {
		return Vector3(x / b.x, y / b.y, z / b.z);
	}

	// Dot product
	// Used to calculate angle q between two vectors among other things,
	// as (A dot B) = |a||b|cos(q).
	T Dot(const Vector3& b) const {
		return x * b.x + y * b.y + z * b.z;
	}

	// Compute cross product, which generates a normal vector.
	// Direction vector can be determined by right-hand rule: Pointing index finder in
	// direction a and middle finger in direction b, thumb will point in a.Cross(b).
	Vector3 Cross(const Vector3& b) const {
		return Vector3(y * b.z - z * b.y, z * b.x - x * b.z, x * b.y - y * b.x);
	}

	// Returns the angle from this vector to b, in radians.
	T Angle(const Vector3& b) const {
		T div = LengthSq() * b.LengthSq();
		OVR_MATH_ASSERT(div != T(0));
		T result = Acos((this->Dot(b)) / sqrt(div));
		return result;
	}

	// Return Length of the vector squared.
	T LengthSq() const {
		return (x * x + y * y + z * z);
	}

	// Return vector length.
	T Length() const {
		return (T)sqrt(LengthSq());
	}

	// Returns squared distance between two points represented by vectors.
	T DistanceSq(Vector3 const& b) const {
		return (*this - b).LengthSq();
	}

	// Returns distance between two points represented by vectors.
	T Distance(Vector3 const& b) const {
		return (*this - b).Length();
	}

	bool IsNormalized() const {
		return fabs(LengthSq() - T(1)) < Math<T>::Tolerance();
	}

	// Normalize, convention vector length to 1.
	void Normalize() {
		T s = Length();
		if (s != T(0))
			s = T(1) / s;
		*this *= s;
	}

	// Returns normalized (unit) version of the vector without modifying itself.
	Vector3 Normalized() const {
		T s = Length();
		if (s != T(0))
			s = T(1) / s;
		return *this * s;
	}

	// Linearly interpolates from this vector to another.
	// Factor should be between 0.0 and 1.0, with 0 giving full value to this.
	Vector3 Lerp(const Vector3& b, T f) const {
		return *this * (T(1) - f) + b * f;
	}

	// Projects this vector onto the argument; in other words,
	// A.Project(B) returns projection of vector A onto B.
	Vector3 ProjectTo(const Vector3& b) const {
		T l2 = b.LengthSq();
		OVR_MATH_ASSERT(l2 != T(0));
		return b * (Dot(b) / l2);
	}

	// Projects this vector onto a plane defined by a normal vector
	Vector3 ProjectToPlane(const Vector3& normal) const {
		return *this - this->ProjectTo(normal);
	}

	bool IsNan() const {
		return !isfinite(x + y + z);
	}
	bool IsFinite() const {
		return isfinite(x + y + z);
	}
};

typedef Vector3<float> Vector3f;
typedef Vector3<double> Vector3d;
typedef Vector3<int32_t> Vector3i;

/*
OVR_MATH_STATIC_ASSERT((sizeof(Vector3f) == 3 * sizeof(float)), "sizeof(Vector3f) failure");
OVR_MATH_STATIC_ASSERT((sizeof(Vector3d) == 3 * sizeof(double)), "sizeof(Vector3d) failure");
OVR_MATH_STATIC_ASSERT((sizeof(Vector3i) == 3 * sizeof(int32_t)), "sizeof(Vector3i) failure");
*/

typedef Vector3<float> Point3f;
typedef Vector3<double> Point3d;
typedef Vector3<int32_t> Point3i;







/******************************
typedefs
******************************/
typedef char GLchar;