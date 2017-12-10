inline OOBB oobb(const Matrix4x4 &tm, const Vector3 &min, const Vector3 &max)
{
	OOBB o;
	o.tm = tm;
	o.min = min;
	o.max = max;
	return o;
}

inline OOBB oobb(const Matrix4x4 &tm)
{
	return oobb(tm, vector3(FLT_MAX, FLT_MAX, FLT_MAX), vector3(-FLT_MAX, -FLT_MAX, -FLT_MAX));
}

inline Vector3 transform(const OOBB &a, const Vector3 &p)
{
	Vector3 s;
	s.x = (1-p.x)*a.min.x + p.x*a.max.x;
	s.y = (1-p.y)*a.min.y + p.y*a.max.y;
	s.z = (1-p.z)*a.min.z + p.z*a.max.z;
	return transform(a.tm, s);
}

inline Vector3 corner(const OOBB &a, unsigned i)
{
	return transform(a, vector3( (float)(i & 1), (float)((i & 2) >> 1), (float)((i & 4) >> 2 )) );
}

inline OOBB extend(const OOBB &a, const Matrix4x4 &tminv, const Vector3 &p)
{
	OOBB o;
	o.tm = a.tm;
	Vector3 local = transform(tminv, p);
	o.max = max(a.max, local);
	o.min = min(a.min, local);
	return o;
}

inline OOBB extend(const OOBB &a, const Matrix4x4 &tminv, const OOBB &b)
{
	OOBB o = a;
	for (unsigned i=0; i<8; ++i)
		o = extend(o, tminv, corner(b, i));
	return o;
}

inline OOBB center(const OOBB &a)
{
	OOBB o = a;
	translation(o.tm) += transform_without_translation(o.tm, (a.min + a.max)/2.0f);
	o.max = (a.max - a.min)/2.0f;
	o.min = -o.max;
	return o;
}

inline bool empty(const OOBB &o)
{
	return (o.min.x > o.max.x || o.min.y > o.max.y || o.min.z > o.max.z);
}
