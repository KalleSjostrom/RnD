using UnityEngine;
using System.Collections;

public class SelectionBuffer {

	public int Size { get; set; }

	private BaseGenome[] members;
	public BaseGenome this[int i]{ get { return members[i]; } set { members[i] = value; } }

	public SelectionBuffer(int size) {
		members = new BaseGenome[size];
		Size = size;
	}
}