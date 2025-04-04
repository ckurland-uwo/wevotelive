<script lang="ts">
    import { Communication } from '$lib/Communication.svelte';
    import GenericSection from '$lib/components/GenericSection.svelte';
    import PollBubble from '$lib/components/PollBubble.svelte';
    import PollButton from '$lib/components/PollButton.svelte';
    import PollDivider from '$lib/components/PollDivider.svelte';
    import PollHeader from '$lib/components/PollHeader.svelte';
    import PollPredictionSegment from '$lib/components/PollPredictionSegment.svelte';
    import PollVoteCard from '$lib/components/PollVoteCard.svelte';
    import PollYouSection from '$lib/components/PollYouSection.svelte';
import '$lib/css/global.css'
    import type { UserVote, WorldState } from '$lib/types';
    import { onMount } from 'svelte';
	import type { PageData } from './$types';
    import { fade, scale } from 'svelte/transition';
    import Errors from '$lib/Errors';
    import { page } from '$app/state';
    import { goto } from '$app/navigation';

	let roomcode: string | null = $state(null)
	let worldState: WorldState = $state({type: "connecting"} as WorldState)

	let communication: Communication | null = $state(null)

	onMount(() => {
		roomcode = page.url.searchParams.get("room")
		if (!roomcode) {
			goto("/")
		}
		communication = new Communication({type: "participant", roomCode: roomcode as string}, (state: WorldState) => {
			worldState = state
		})
		communication.begin()
		return ()=> {
			communication?.end()
		}
	})

	$inspect(worldState)
</script>

<main>
	<div class="padding"></div>
	{#if roomcode}
		<PollBubble>
			<GenericSection>
				<h1>Room {roomcode}</h1>
			</GenericSection>
		</PollBubble>
	{/if}

	{#if worldState.type == "participant"}
		{#each worldState.polls as poll (poll.id)}
			<PollBubble>
				<PollHeader poll={poll} />
				{#if !worldState.votes[poll.id] && !poll.revealed}
					<PollVoteCard poll={poll} communication={communication as Communication} />
				{/if}
				<PollDivider/>
				<PollPredictionSegment poll={poll} />
				{#if worldState.votes[poll.id]}
					<PollDivider/>
					<PollYouSection poll={poll} vote={worldState.votes[poll.id] as UserVote} />
				{/if}
			</PollBubble>
		{:else}
			<PollBubble>
				<GenericSection>
					<p style="text-align: center">No polls yet, hang tight!</p>
				</GenericSection>
			</PollBubble>
		{/each}
	{:else if worldState.type == "connecting"}
		<PollBubble>
			<GenericSection>
				<div class="connect-background"></div>
				<h1>Connecting...</h1>
			</GenericSection>
		</PollBubble>
	{:else if worldState.type == "error"}
		<PollBubble>
			<GenericSection>
				<div class="error-background"></div>
				<h1>An error occured</h1>
				<p style="text-align: center;">{Errors[worldState.error]}</p>
				{#if worldState.retry}
					<p class="reconnect-text">We're reconnecting, just a moment!</p>
				{/if}
			</GenericSection>
		</PollBubble>
	{/if}
	<div class="padding"></div>
</main>

<style>
	main {
		margin: auto;
		padding: 20px 20px;
		max-width: 600px;

		display: flex;
		flex-direction: column;
		min-height: 100vh;
		box-sizing: border-box;
		justify-content: center;

		gap: 10px;
	}
	h1 {
		margin: 0px;
		text-align: center;
	}

	.connect-background {
		background-color: rgb(115, 0, 255);
		position: absolute;
		inset: 0;
		left: 50px;
		right: 50px;
		z-index: -1;
		filter: blur(100px);
	}
	.error-background {
		background-color: rgb(255, 0, 81);
		position: absolute;
		inset: 0;
		left: 50px;
		right: 50px;
		z-index: -1;
		filter: blur(100px);
	}
	.reconnect-text {
		text-align: center;
		font-weight: bold;
		margin: 5px;
		font-style: italic;
	}

	.padding {
		height: 100px;
	}
</style>
