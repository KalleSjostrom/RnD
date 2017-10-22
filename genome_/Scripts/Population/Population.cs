using UnityEngine;
using System.Collections;

public class Population {

	public int Size { get; set; }

	public float MinFitness { get; set; }
	public float MaxFitness { get; set; }
	public float MeanFitness { get { return TotalFitness / Size; } }
	public float TotalFitness { get; set; }
	public float MedianFitness { get; set; }

	private PhenomeDescription[] members;
	public PhenomeDescription this[int i]{ get { return members[i]; } set { members[i] = value; } }

	public Population(int size) {
		members = new PhenomeDescription[size];
		Size = size;
	}

	public void Sort() {
		System.Array.Sort(members);
	}
}