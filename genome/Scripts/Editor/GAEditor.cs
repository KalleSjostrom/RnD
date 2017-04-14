using UnityEngine;
using UnityEditor;
using System.Collections;
using System.Collections.Generic;

namespace AGP {
	public interface IEditorItem {
	}
	public interface IDragable {
		void MoveTo(Vector2 position);
	}
	public enum Command {
		Move,
		Cut,
		Copy,
		Paste,
		CreateEdge,
		CreateNode,
	}

	public interface ICommand {
		void UndoCommand();
	}

	public class DragCommand : ICommand {
		Vector2 from;
		Vector2 to;
		IDragable dragedItem;

		public DragCommand(Vector2 from, Vector2 to, IDragable dragedItem) {
			this.from = from;
			this.to = to;
			this.dragedItem = dragedItem;
		}
		public void UndoCommand() {
			dragedItem.MoveTo(from);
		}
	}

	public class AddNewItemCommand : ICommand {
		IEditorItem item;
		
		public AddNewItemCommand(IEditorItem item) {
			this.item = item;
		}
		public void UndoCommand() {
			// remove item
		}
	}

	public interface IEditorState
	{
		void OnEnter(GAEditor editor);
		void OnUpdate(GAEditor editor);
		void OnExit(GAEditor editor);
	}

	public class IdleState : IEditorState {
		public void OnEnter(GAEditor editor) {}
		public void OnUpdate(GAEditor editor) {}
		public void OnExit(GAEditor editor) {}
	}

	public class DragState : IEditorState {
		private Vector2 startPosition;
		private IDragable dragedItem;
		public void OnEnter(GAEditor editor) {
			startPosition = Input.mousePosition;
			dragedItem = editor.ActiveItem as IDragable;
		}
		public void OnUpdate(GAEditor editor) {
			if (Input.GetMouseButtonUp(0))
				editor.ChangeStateTo(State.Idle);
		}
		public void OnExit(GAEditor editor) {
			Vector3 endPosition = Input.mousePosition;
			editor.AddToHistory(new DragCommand(startPosition, endPosition, dragedItem));
		}
	}

	public enum State {
		Idle,
		Drag,
	}

	public class GAEditor {

		public IEditorItem ActiveItem { get; set; }
		public IEditorState CurrentState { get { return currentState; } }

		private Dictionary<State, IEditorState> states = new Dictionary<State, IEditorState>();
		private Stack<ICommand> history;
		private IEditorState currentState;
		private Graph graph;

		public GAEditor(Graph graph) {
			this.graph = graph;

			states.Add(State.Idle, new IdleState());
			states.Add(State.Drag, new DragState());
			history = new Stack<ICommand>();

			ChangeStateTo(State.Idle);
		}

		public void AddToHistory(ICommand command) {
			history.Push(command);
		}

		public void UndoCommand() {
			ICommand command = history.Pop();
			command.UndoCommand();
		}

		public void ChangeStateTo(State nextState) {
			if (currentState != null)
				currentState.OnExit(this);
			
			currentState = states[nextState];
			currentState.OnEnter(this);
		}
	}

	/*public class CommandHandler {

		public void ApplyCommand(Command command) {
			switch (command) {
			case Command.CreateEdge : 
				new Edge(source);
				break;
			default: Debug.LogError("Not implemented"); break;
			}
		}
	}*/
}
