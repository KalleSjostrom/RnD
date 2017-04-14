using UnityEngine;
using UnityEditor;
using System.Collections;

namespace AGP {
	public class Node : IEditorItem
	{
		private Rect rect = new Rect (100 + 100, 100, 100, 100);
		public Rect Rectangle { get { return rect; } }

		NodeConnection[] nodecs;

		public Node() {
			nodecs = new NodeConnection[10];
			for (int i = 0; i < nodecs.Length; i++)
				nodecs[i] = new NodeConnection(this, (i / 5), (i % 5));
		}

		public void Draw(EditorWindow main, GAEditor editor) {
			main.BeginWindows();
			rect = GUI.Window(0, rect, DrawWindow, "Box1");
			main.EndWindows();
			int size = GUI.skin.button.fontSize;
			GUI.skin.button.fontSize = 10;

			for (int i = 0; i < nodecs.Length; i++)
				nodecs[i].Draw(main, editor);
			GUI.skin.button.fontSize = size;
		}

		void DrawWindow(int windowID) {
			if (GUI.Button(new Rect(20, 50, 50, 50), "Hello")) {
				Debug.Log("HELLO");
			}
			GUI.Toggle(new Rect(70, 50, 50, 50), false, "Doh");
			GUI.DragWindow();
		}
	}
}