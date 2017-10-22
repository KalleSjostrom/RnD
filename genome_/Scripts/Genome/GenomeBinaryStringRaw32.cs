using UnityEngine;
using System.Collections;

public class GenomeBinaryStringRaw32 : BaseGenomeBinary {

	private uint bits;

	public GenomeBinaryStringRaw32(int size) {
		bits = (uint)(Random.value * 0xFFFFFFFF);
		Length = size;
	}

	public override BaseGenome CreateRandom() {
		return new GenomeBinaryStringRaw32(Length);
	}
	public override void CloneFrom(BaseGenome a) {
		bits = (a as GenomeBinaryStringRaw32).bits;
	}

	public override void Set(int index, bool isSet) {
		uint temp = (uint)(1 << index);
		bits = isSet ? (bits | temp) : (bits & ~temp);
	}
	
	public override bool IsSet(int index) {
		uint temp = (uint)1 << index;
		return (bits & temp) == temp;
	}
	
	public override void Flip(int index) {
		uint temp = (uint)1 << index;
		bits ^= temp;
	}

	public override void OnePointCrossover(BaseGenome mom, BaseGenome dad, int point) {
		GenomeBinaryStringRaw32 m = mom as GenomeBinaryStringRaw32;
		GenomeBinaryStringRaw32 d = dad as GenomeBinaryStringRaw32;
		uint mask = 0xFFFFFFFF << point;
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
