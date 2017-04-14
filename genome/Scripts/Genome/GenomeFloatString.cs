using UnityEngine;
using System.Collections;

public class GenomeFloatString : BaseGenome {

	private float[] floats;
	public override int Length { get { return floats.Length; } }

	public GenomeFloatString(int size) {
		floats = new float[size];
	}
	public GenomeFloatString(float[] floats) {
		this.floats = floats;
	}

	public override BaseGenome CreateRandom() {
		float[] random = new float[Length];
		for (int i = 0; i < random.Length; i++) {
			random[i] = Random.Range(-1.0f, 1.0f); // TODO: Should be possible to specify this!
		}
		return new GenomeFloatString(random);
	}

	public override void CloneFrom(BaseGenome source) {
		GenomeFloatString s = source as GenomeFloatString;
		for (int i = 0; i < floats.Length; i++)
			floats[i] = s.floats[i];
	}

	public override void OnePointCrossover(BaseGenome mom, BaseGenome dad, int point) {
		GenomeFloatString m = mom as GenomeFloatString;
		GenomeFloatString d = dad as GenomeFloatString;
		for (int i = 0; i < floats.Length; i++) {
			floats[i] = (i < point ? m : d).floats[i];
		}
	}

	public override void Modify(int point) {
	}
	public override void Switch(int pointa, int pointb) {
	}

	public float[] GetArray() {
		return floats;
	}
}
