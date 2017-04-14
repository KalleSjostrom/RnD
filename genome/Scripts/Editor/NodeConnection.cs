using UnityEngine;
using UnityEditor;
using System.Collections;

namespace AGP {
	public class NodeConnection : IEditorItem
	{
		// private static Vector3 OFFSETS = {};
		private Rect rect;
		public Rect Rectangle { get { return rect; } }
		public Vector3 Direction { get { return new Vector3(1, 0, 0); } }

		private Node parent;
		private Edge edge;
		private Vector2 offset;

		private static int WIDTH = 40;
		private static int HEIGHT = 18;
		private static int HALF_WIDTH = WIDTH/2;
		private static int HALF_HEIGHT = HEIGHT/2;

		public NodeConnection(Node parent, int side, int index) {
			this.parent = parent;
			rect = new Rect(0, 0, WIDTH, HEIGHT);

			Rect parentRect = parent.Rectangle;

			float w = parentRect.width;
			float h = parentRect.height;

			float x = (side == 0 ? 1 : -1) * (w/2 + HALF_WIDTH);
			float y = -h/2 + (HEIGHT+1) * (index+1);
			offset = new Vector2(x, y);
		}
		
		public void Draw(EditorWindow main, GAEditor editor) {
			Rect parentRect = parent.Rectangle;
			rect.center = parentRect.center + offset;

			if (GUI.Button(rect, "Mate")) {
				edge = new Edge(this);
			}

			if (edge != null) {
				edge.Draw(main, editor);
			}
		}
	}
}
