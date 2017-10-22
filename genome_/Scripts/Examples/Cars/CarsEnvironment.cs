using UnityEngine;
using System.Collections;

public class CarsEnvironment : SimulationEnvironment {

	protected override float CalculateFitness(BasePhenome phenome) {
		return (phenome as CarPhenome).TotalDistance;
	}
}
