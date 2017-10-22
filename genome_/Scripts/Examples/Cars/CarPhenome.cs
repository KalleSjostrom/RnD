using UnityEngine;
using System.Collections;
using System;

public class CarPhenome : BasePhenome {

	private static Vector2[] DIRECTIONS = { 
		new Vector2(0, 1), Vector3.Normalize(new Vector2(1, 1)), 
		new Vector2(1, 0), Vector3.Normalize(new Vector2(1, -1)), new Vector2(0, -1) 
	};
	private static Collider2D[] CACHE = new Collider2D[1];

	public float speed = 1;
	private float[] sensors;
	private float totalDistance;
	private NeuralNet neuralNet;
	public float TotalDistance { get { return totalDistance; } }
	
	public override void Init(BaseGenome genome) {
		base.Init(genome);

		GenomeFloatString gfs = genome as GenomeFloatString;
		neuralNet = new NeuralNet(new int[]{5, 8, 3}, gfs.GetArray());

		sensors = new float[5+1];
		sensors[5] = 0;
		totalDistance = 0;
	}
	
	protected override void UpdatePhenome() {
		Feelers.Feel(transform, DIRECTIONS, 0.5f, sensors, true);

		float[] outputs = neuralNet.Evaluate(sensors);

		float leftTheta = outputs[0] * 6;
		float rightTheta = outputs[1] * 6;
		float speed = outputs[2] * 6;

		float dt = Time.fixedDeltaTime;

		float angle = transform.rotation.eulerAngles.z;
		angle += (leftTheta - rightTheta) * dt * 200;

		float x = Mathf.Cos(Mathf.Deg2Rad * angle);
		float y = Mathf.Sin(Mathf.Deg2Rad * angle);
		Vector3 heading = new Vector3(x, y, 0);

		heading *= speed * dt;
		totalDistance += Vector3.Magnitude(heading);
		transform.position += heading;
		gameObject.transform.rotation = Quaternion.Euler(new Vector3(0, 0, angle));

		Vector2 pos = new Vector2(transform.position.x, transform.position.y);
		CACHE[0] = null;
		Physics2D.OverlapCircleNonAlloc(pos, 0.1f, CACHE);
		if (CACHE[0] != null) {
			// Debug.Log(leftTheta + " " + rightTheta + " " + speed);
			SetAsDone();
		}

		if (totalDistance > 25)
			SetAsDone();
	}
}
