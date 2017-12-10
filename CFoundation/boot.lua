function init()
	-- CORE FLOW NODES
	-- require 'core/animation/lua/runtime/animationflowcallbacks' -- Needed for core/animation/flow/animation.script_flow_nodes
	-- TODO(kalle): The appkit flow nodes are unuseable since we aren't using it.
	-- We want to not load this at all so they won't show up in the level editor. Don't know how we can avoid loading the appkit.script_flow_nodes though..
	-- require 'core/appkit/lua/appkit_flow_callbacks' -- Needed for core/appkit/appkit.script_flow_nodes
	-- require 'core/gwnav/lua/runtime/navflowcallbacks' -- Needed for core/gwnav/flow/gwnav.script_flow_nodes"
	require 'core/humanik/lua/humanik_flow_callbacks' -- Needed for core/humanik/humanik.script_flow_nodes"
	-- require 'core/scaleform_studio/lua/scaleform_studio_flow_callbacks' -- Needed for core/scaleform_studio/scaleform_studio.script_flow_nodes"
	require 'core/wwise/lua/wwise_flow_callbacks' -- Needed for core/wwise/wwise.script_flow_nodes
	------------------
	-- require "script/lua/flow_callbacks"

	stingray.Application.set_time_step_policy("throttle", 100)
	stingray.Script.configure_garbage_collection(stingray.Script.MINIMUM_COLLECT_TIME_MS, 0.0)
end

function shutdown()
end

function update(dt)
end

function render()
end
