using UnityEngine;
using System.Collections;

public class Feelers {

	public static void Feel(Transform transform, Vector2[] directions, float lenght, float[] output, bool debugDraw) {
		Vector3 pos = transform.position;
		Quaternion q = transform.rotation;
		
		for (int i = 0; i < directions.Length; i++) {
			Vector2 dir = q * directions[i];
			RaycastHit2D hit = Physics2D.Raycast(pos, dir, lenght);
			
			dir = dir*lenght;
			bool anyHit = hit.collider != null;
			float fraction = anyHit ? hit.fraction : 1;
			output[i] = 1-fraction;

			if (debugDraw) {
				if (anyHit) {
					Debug.DrawRay(pos, dir, Color.black);
					Debug.DrawRay(pos, dir * fraction, Color.red);
				} else {
					Debug.DrawRay(pos, dir, Color.green);
				}
			}
		}
	}
}
