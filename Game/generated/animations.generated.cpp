namespace animation {
	struct Data {
		int keyframe_count;
		v3 *keyframes;

		int stride;
	};
	inline Data make_data(v3 *keyframes, int keyframe_count, int stride) {
		Data d = { keyframe_count, keyframes, stride };
		return d;
	}
	namespace walk {
		int stride = 61;
		v3 keyframes[] = {
			{0.000000, 1.000000, 0.000000},
			{0.000000, 1.000000, 0.000000},
			{0.000000, 1.000000, 0.000000},
			{0.000000, 1.000000, 0.000000},
			{0.000000, 1.000000, 0.000000},
			{0.000000, 1.000000, 0.000000},
			{0.000000, 1.000000, 0.000000},
			{0.000000, 1.000000, 0.000000},
			{0.000000, 1.000000, 0.000000},
			{0.000000, 1.000000, 0.000000},
			{0.000000, 1.000000, 0.000000},
			{0.000000, 1.000000, 0.000000},
			{0.000000, 1.000000, 0.000000},
			{0.000000, 1.000000, 0.000000},
			{0.000000, 1.000000, 0.000000},
			{0.000000, 1.000000, 0.000000},
			{0.000000, 1.000000, 0.000000},
			{0.000000, 1.000000, 0.000000},
			{0.000000, 1.000000, 0.000000},
			{0.000000, 1.000000, 0.000000},
			{0.000000, 1.000000, 0.000000},
			{0.000000, 1.000000, 0.000000},
			{0.000000, 1.000000, 0.000000},
			{0.000000, 1.000000, 0.000000},
			{0.000000, 1.000000, 0.000000},
			{0.000000, 1.000000, 0.000000},
			{0.000000, 1.000000, 0.000000},
			{0.000000, 1.000000, 0.000000},
			{0.000000, 1.000000, 0.000000},
			{0.000000, 1.000000, 0.000000},
			{0.000000, 1.000000, 0.000000},
			{0.000000, 1.000000, 0.000000},
			{0.000000, 1.000000, 0.000000},
			{0.000000, 1.000000, 0.000000},
			{0.000000, 1.000000, 0.000000},
			{0.000000, 1.000000, 0.000000},
			{0.000000, 1.000000, 0.000000},
			{0.000000, 1.000000, 0.000000},
			{0.000000, 1.000000, 0.000000},
			{0.000000, 1.000000, 0.000000},
			{0.000000, 1.000000, 0.000000},
			{0.000000, 1.000000, 0.000000},
			{0.000000, 1.000000, 0.000000},
			{0.000000, 1.000000, 0.000000},
			{0.000000, 1.000000, 0.000000},
			{0.000000, 1.000000, 0.000000},
			{0.000000, 1.000000, 0.000000},
			{0.000000, 1.000000, 0.000000},
			{0.000000, 1.000000, 0.000000},
			{0.000000, 1.000000, 0.000000},
			{0.000000, 1.000000, 0.000000},
			{0.000000, 1.000000, 0.000000},
			{0.000000, 1.000000, 0.000000},
			{0.000000, 1.000000, 0.000000},
			{0.000000, 1.000000, 0.000000},
			{0.000000, 1.000000, 0.000000},
			{0.000000, 1.000000, 0.000000},
			{0.000000, 1.000000, 0.000000},
			{0.000000, 1.000000, 0.000000},
			{0.000000, 1.000000, 0.000000},
			{0.000000, 1.000000, 0.000000},
			{0.000000, 11.000000, 0.000000},
			{-1.045280, 10.945220, 0.000000},
			{-2.079120, 10.781480, 0.000000},
			{-3.090170, 10.510571, 0.000000},
			{-4.067370, 10.135450, 0.000000},
			{-5.000000, 9.660250, 0.000000},
			{-5.877850, 9.090170, 0.000000},
			{-6.691310, 8.431450, 0.000000},
			{-7.431450, 7.691310, 0.000000},
			{-8.090170, 6.877850, 0.000000},
			{-8.660250, 6.000000, 0.000000},
			{-9.135450, 5.067370, 0.000000},
			{-9.510571, 4.090170, 0.000000},
			{-9.781480, 3.079120, 0.000000},
			{-9.945220, 2.045280, 0.000000},
			{-10.000000, 1.000000, 0.000000},
			{-9.945220, -0.045280, 0.000000},
			{-9.781480, -1.079120, 0.000000},
			{-9.510571, -2.090170, 0.000000},
			{-9.135450, -3.067370, 0.000000},
			{-8.660250, -4.000000, 0.000000},
			{-8.090170, -4.877850, 0.000000},
			{-7.431450, -5.691310, 0.000000},
			{-6.691310, -6.431450, 0.000000},
			{-5.877850, -7.090170, 0.000000},
			{-5.000000, -7.660250, 0.000000},
			{-4.067370, -8.135450, 0.000000},
			{-3.090170, -8.510571, 0.000000},
			{-2.079120, -8.781480, 0.000000},
			{-1.045280, -8.945220, 0.000000},
			{0.000000, -9.000000, 0.000000},
			{1.045280, -8.945220, 0.000000},
			{2.079120, -8.781480, 0.000000},
			{3.090170, -8.510571, 0.000000},
			{4.067370, -8.135450, 0.000000},
			{5.000000, -7.660250, 0.000000},
			{5.877850, -7.090170, 0.000000},
			{6.691310, -6.431450, 0.000000},
			{7.431450, -5.691310, 0.000000},
			{8.090170, -4.877850, 0.000000},
			{8.660250, -4.000000, 0.000000},
			{9.135450, -3.067370, 0.000000},
			{9.510571, -2.090170, 0.000000},
			{9.781480, -1.079120, 0.000000},
			{9.945220, -0.045280, 0.000000},
			{10.000000, 1.000000, 0.000000},
			{9.945220, 2.045280, 0.000000},
			{9.781480, 3.079120, 0.000000},
			{9.510571, 4.090170, 0.000000},
			{9.135450, 5.067370, 0.000000},
			{8.660250, 6.000000, 0.000000},
			{8.090170, 6.877850, 0.000000},
			{7.431450, 7.691310, 0.000000},
			{6.691310, 8.431450, 0.000000},
			{5.877850, 9.090170, 0.000000},
			{5.000000, 9.660250, 0.000000},
			{4.067370, 10.135450, 0.000000},
			{3.090170, 10.510571, 0.000000},
			{2.079120, 10.781480, 0.000000},
			{1.045280, 10.945220, 0.000000},
			{0.000000, 11.000000, 0.000000},
			{5.000000, 11.000000, 0.000000},
			{3.927331, 10.422574, 0.000000},
			{2.811619, 9.741920, 0.000000},
			{1.665114, 8.965486, 0.000000},
			{0.500359, 8.101770, 0.000000},
			{-0.669875, 7.160254, 0.000000},
			{-1.832765, 6.151241, 0.000000},
			{-2.975585, 5.085794, 0.000000},
			{-4.085794, 3.975585, 0.000000},
			{-5.151241, 2.832765, 0.000000},
			{-6.160254, 1.669875, 0.000000},
			{-7.101770, 0.499641, 0.000000},
			{-7.965485, -0.665114, 0.000000},
			{-8.741920, -1.811619, 0.000000},
			{-9.422574, -2.927331, 0.000000},
			{-10.000000, -4.000000, 0.000000},
			{-10.467866, -5.017891, 0.000000},
			{-10.821040, -5.969858, 0.000000},
			{-11.055655, -6.845454, 0.000000},
			{-11.169130, -7.635099, 0.000000},
			{-11.160246, -8.330125, 0.000000},
			{-11.029099, -8.922935, 0.000000},
			{-10.777105, -9.407035, 0.000000},
			{-10.407035, -9.777105, 0.000000},
			{-9.922935, -10.029099, 0.000000},
			{-9.330125, -10.160246, 0.000000},
			{-8.635099, -10.169130, 0.000000},
			{-7.845454, -10.055655, 0.000000},
			{-6.969858, -9.821040, 0.000000},
			{-6.017891, -9.467866, 0.000000},
			{-5.000000, -9.000000, 0.000000},
			{-3.927331, -8.422574, 0.000000},
			{-2.811619, -7.741920, 0.000000},
			{-1.665114, -6.965485, 0.000000},
			{-0.500359, -6.101770, 0.000000},
			{0.669875, -5.160254, 0.000000},
			{1.832765, -4.151241, 0.000000},
			{2.975585, -3.085794, 0.000000},
			{4.085794, -1.975585, 0.000000},
			{5.151241, -0.832765, 0.000000},
			{6.160254, 0.330125, 0.000000},
			{7.101770, 1.500359, 0.000000},
			{7.965485, 2.665114, 0.000000},
			{8.741920, 3.811619, 0.000000},
			{9.422574, 4.927331, 0.000000},
			{10.000000, 6.000000, 0.000000},
			{10.467866, 7.017891, 0.000000},
			{10.821040, 7.969858, 0.000000},
			{11.055655, 8.845454, 0.000000},
			{11.169130, 9.635099, 0.000000},
			{11.160246, 10.330125, 0.000000},
			{11.029099, 10.922935, 0.000000},
			{10.777105, 11.407035, 0.000000},
			{10.407035, 11.777105, 0.000000},
			{9.922935, 12.029099, 0.000000},
			{9.330125, 12.160246, 0.000000},
			{8.635099, 12.169130, 0.000000},
			{7.845454, 12.055655, 0.000000},
			{6.969858, 11.821040, 0.000000},
			{6.017891, 11.467866, 0.000000},
			{5.000000, 11.000000, 0.000000},
			{2.000000, 0.000000, 0.000000},
			{2.070946, 0.429805, 0.000000},
			{2.074200, 0.861130, 0.000000},
			{2.010464, 1.284566, 0.000000},
			{1.881663, 1.690902, 0.000000},
			{1.690907, 2.071261, 0.000000},
			{1.442438, 2.417257, 0.000000},
			{1.141554, 2.721129, 0.000000},
			{0.794535, 2.975864, 0.000000},
			{0.408530, 3.175321, 0.000000},
			{-0.008547, 3.314335, 0.000000},
			{-0.448156, 3.388805, 0.000000},
			{-0.901264, 3.395764, 0.000000},
			{-1.358479, 3.333436, 0.000000},
			{-1.810219, 3.201277, 0.000000},
			{-2.246855, 3.000000, 0.000000},
			{-2.381150, 3.032713, 0.000000},
			{-2.547587, 3.056210, 0.000000},
			{-2.745385, 3.064078, 0.000000},
			{-2.972774, 3.049991, 0.000000},
			{-3.227005, 3.007873, 0.000000},
			{-3.504386, 2.932001, 0.000000},
			{-3.800330, 2.817151, 0.000000},
			{-4.109415, 2.658731, 0.000000},
			{-4.425484, 2.452875, 0.000000},
			{-4.741732, 2.196568, 0.000000},
			{-5.050818, 1.887719, 0.000000},
			{-5.344992, 1.525239, 0.000000},
			{-5.616211, 1.109126, 0.000000},
			{-5.856307, 0.640468, 0.000000},
			{-6.057119, 0.121514, 0.000000},
			{-5.833232, -0.496420, 0.000000},
			{-5.547608, -1.077294, 0.000000},
			{-5.205525, -1.615072, 0.000000},
			{-4.812825, -2.104433, 0.000000},
			{-4.375855, -2.540790, 0.000000},
			{-3.901338, -2.920355, 0.000000},
			{-3.396291, -3.240155, 0.000000},
			{-2.867943, -3.498049, 0.000000},
			{-2.323615, -3.692752, 0.000000},
			{-1.770649, -3.823823, 0.000000},
			{-1.216285, -3.891653, 0.000000},
			{-0.667578, -3.897439, 0.000000},
			{-0.131330, -3.843139, 0.000000},
			{0.386031, -3.731454, 0.000000},
			{0.878486, -3.565761, 0.000000},
			{1.208498, -3.291899, 0.000000},
			{1.492447, -3.000790, 0.000000},
			{1.730506, -2.698891, 0.000000},
			{1.923688, -2.392412, 0.000000},
			{2.073772, -2.087229, 0.000000},
			{2.183259, -1.788777, 0.000000},
			{2.255286, -1.501970, 0.000000},
			{2.293546, -1.231133, 0.000000},
			{2.302204, -0.979943, 0.000000},
			{2.285801, -0.751379, 0.000000},
			{2.249150, -0.547688, 0.000000},
			{2.197231, -0.370354, 0.000000},
			{2.135084, -0.220107, 0.000000},
			{2.067720, -0.096912, 0.000000},
			{2.000000, 0.000000, 0.000000},
		};
		Data data = make_data(keyframes, ARRAY_COUNT(keyframes), stride);
	};
};
