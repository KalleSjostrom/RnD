using UnityEngine;
using System.Collections;

public class GenomeBinaryString : BaseGenomeBinary {

	private BitArray bits;
	public override int Length { get { return bits.Length; } }

	public GenomeBinaryString(int size) {
		bits = new BitArray(size);
	}

	public override BaseGenome CreateRandom() {
		GenomeBinaryString random = new GenomeBinaryString(Length);
		for (int i = 0; i < random.Length; i++) {
			random.Set(i, Random.value < 0.5f);
		}
		return random;
	}

	public override void Set(int index, bool isSet) {
		bits[index] = isSet;
	}

	public override bool IsSet(int index) {
		return bits[index];
	}

	public override void Flip(int index) {
		bits[index] = !bits[index];
	}

	public override void CloneFrom(BaseGenome source) {
		GenomeBinaryString s = source as GenomeBinaryString;
		for (int i = 0; i < bits.Length; i++)
			bits[i] = s.bits[i];
	}

	public override void OnePointCrossover(BaseGenome mom, BaseGenome dad, int point) {
		GenomeBinaryString m = mom as GenomeBinaryString;
		GenomeBinaryString d = dad as GenomeBinaryString;
		for (int i = 0; i < bits.Length; i++) {
			bits[i] = (i < point ? m : d).bits[i];
		}
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
