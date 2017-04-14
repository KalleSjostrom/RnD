using UnityEngine;
using System.Collections;

public class GASettings : MonoBehaviour {

	public int NumberOfGenerations = 1000;
	public int PopulationSize = 1000;
	public bool RunAsFastAsPossible = false;
	public float SelectionProportion = 0.5f;

	public bool TerminateSolutionFound = false;
	public float WantedFitness = 1;
}
