using UnityEngine;
using System.Collections;

public abstract class AMutationStrategy : MonoBehaviour {

	public GASettings Settings { get; set; }

	public abstract void Mutate(Population population, GeneticAlgorithm.NextStepDelegate callback);
}
