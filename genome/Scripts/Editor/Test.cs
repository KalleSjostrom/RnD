using UnityEngine;
using UnityEditor;
using System.Collections;
using AGP;

public class GraphEditorWindow : EditorWindow
{
	private GAEditor editor;
	private Graph graph;
	private Node n1;
	private Node n2;

	/*
	 * The MenuItem attribute allows you to add menu items to the main menu and inspector context menus.

The MenuItem attribute turns any static function into a menu command. Only static functions can use the MenuItem attribute.
To create a hotkey you can use the following special characters: % (ctrl on Windows, cmd on OS X), # (shift), & (alt), <b>_</b> (no key modifiers). For example to create a menu with hotkey shift-alt-g use "MyMenu/Do Something #&g". To create a menu with hotkey g and no key modifiers pressed use "MyMenu/Do Something _g".
Some special keyboard keys are supported as hotkeys, for example "#LEFT" would map to shift-left. The keys supported like this are: LEFT, RIGHT, UP, DOWN, F1 .. F12, HOME, END, PGUP, PGDN.
A hotkey text must be preceded with a space character ("MyMenu/Do_g" won't be interpreted as hotkey, while "MyMenu/Do _g" will).*/


	/*
	 * public void OnSceneGUI() {
   Event e &#61; Event.current;
   if (e.control && e.type == EventType.KeyDown && e.keyCode == KeyCode.Z) {
      //Undo process;
      e.Use();
   }
   if (e.control && e.type == EventType.KeyDown && e.keyCode == KeyCode.Y) {
      //Redo process;
      e.Use();
   }
}*/

	[MenuItem ("Window/Graph Editor Window")]
	static void Init() {
		GraphEditorWindow gew = EditorWindow.GetWindow<GraphEditorWindow>();
		gew.Setup();
	}

	public void Testing() {
	}

	void OnDestroy() {
		Undo.undoRedoPerformed -= this.Testing;
	}

	void Setup() {
		graph = new Graph();
		editor = new GAEditor(graph);

		Undo.RecordObject(this, "Testing");
		Undo.undoRedoPerformed += this.Testing;

		Node n = new Node();
		editor.AddToHistory(new AddNewItemCommand(n));
		graph.AddNode(n);
	}
	
	private void OnGUI()
	{
		GUI.Label(new Rect(10, 10, 100, 50), editor.CurrentState.ToString());
		graph.Draw(this, editor);
	}
}
