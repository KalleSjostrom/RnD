class TestOne {
	virtual void doit() = 0;
};

class Blah {
	int a;
};


namespace blaha {
	class Blah {
		int a;
	};
}

class TestTwo : TestOne {
	Blah test_array[32];
	blaha::Blah test_array2[32];

	virtual void doit() {
	}
	void doit2() {

	}
};
