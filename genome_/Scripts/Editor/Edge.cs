using UnityEngine;
using UnityEditor;
using System.Collections;

namespace AGP {
	public class Edge : IEditorItem
	{
		private NodeConnection source;
		private NodeConnection destination;

		public Edge(NodeConnection source) {
			this.source = source;
		}

		public Edge(NodeConnection source, NodeConnection destination) {
			this.source = source;
			this.destination = destination;
		}

		public void Draw(EditorWindow main, GAEditor editor) {
			Handles.BeginGUI();

			if (destination == null) {
				Rect r1 = source.Rectangle;

				Vector3 p1 = r1.center;
				Vector3 p2 = Input.mousePosition;
				Vector3 p3 = p1 + source.Direction * 20;
				Vector3 p4 = p2;

				Handles.DrawBezier(p1, p2, p3, p4, Color.red, null, 5f);
			} else {
				Rect r1 = source.Rectangle;
				Rect r2 = destination.Rectangle;

				Vector3 p1 = r1.center;
				Vector3 p2 = r2.center;
				Vector3 p3 = p1 + source.Direction * 20;
				Vector3 p4 = p2 + destination.Direction * 20;
				Handles.DrawBezier(p1, p2, p3, p4, Color.red, null, 5f);
			}
			
			
			Handles.EndGUI();
		}
	}
}