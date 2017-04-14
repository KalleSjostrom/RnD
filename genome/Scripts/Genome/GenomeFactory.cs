using UnityEngine;
using System.Collections;

public enum GenomeType {
	RawBinaryString32,
	RawBinaryString64,
	BinaryString,
	FloatString,
}

public static class GenomeFactory {
	
	public static BaseGenome CreateGenome(GenomeType type, int length) {
		switch (type) {
		case GenomeType.BinaryString: return new GenomeBinaryString(length);
		case GenomeType.RawBinaryString32: return new GenomeBinaryStringRaw32(length);
		case GenomeType.RawBinaryString64: return new GenomeBinaryStringRaw64(length);
		case GenomeType.FloatString: return new GenomeFloatString(length);
		}
		DebugAux.Assert(false, "There is no such genome type");
		return null;
	}
}
