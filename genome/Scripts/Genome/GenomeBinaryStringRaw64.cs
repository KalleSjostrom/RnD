using UnityEngine;
using System.Collections;

public class GenomeBinaryStringRaw64 : BaseGenomeBinary {

	private ulong bits;
	private static ulong AllOnes = 2^64;

	public GenomeBinaryStringRaw64(int size) {
		bits = (ulong)(Random.value * AllOnes);
		Length = size;
	}

	public override BaseGenome CreateRandom() {
		return new GenomeBinaryStringRaw64(Length);
		/*b.bits = (ulong)(Random.value * AllOnes);
		return b;*/
	}
	public override void CloneFrom(BaseGenome a) {
		bits = (a as GenomeBinaryStringRaw64).bits;
	}

	public override void Set(int index, bool isSet) {
		ulong temp = (ulong)1 << index;
		bits = isSet ? (bits | temp) : (bits & ~temp);
	}
	
	public override bool IsSet(int index) {
		ulong temp = (ulong)1 << index;
		return (bits & temp) == temp;
	}
	
	public override void Flip(int index) {
		ulong temp = (ulong)1 << index;
		bits ^= temp;
	}

	public override void OnePointCrossover(BaseGenome mom, BaseGenome dad, int point) {
		GenomeBinaryStringRaw64 m = mom as GenomeBinaryStringRaw64;
		GenomeBinaryStringRaw64 d = dad as GenomeBinaryStringRaw64;
		ulong mask = AllOnes << point;
		bits = (mask & m.bits) | (~mask & d.bits);
	}
	public override void Modify(int point) {
		Flip(point);
	}
	public override void Switch(int point1, int point2) {
		bool b1 = IsSet(point1);
		bool b2 = IsSet(point2);
		Set(point2, b1);
		Set(point1, b2);
	}
}
