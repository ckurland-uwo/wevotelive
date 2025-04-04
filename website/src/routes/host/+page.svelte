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
	import QRCode from 'qrcode'
	import Errors from '$lib/Errors'

	// let { data }: { data: PageData } = $props();

	let worldState: WorldState = $state({type: "connecting"} as WorldState)

	const communication = new Communication({type: "host"}, (state: WorldState) => {
		worldState = state
	})

	onMount(() => {
		communication.begin()
		return ()=> {
			communication.end()
		}
	})

	function qrCode(node: HTMLElement, room: string) {
		QRCode.toString(`http://wevotelive.azurestudios.ca/join/?room=${room}`, {type: "svg", margin: 0, color: {light: "#ffffff00", dark: "#2c1849"}}, (err: any, data: string)=>{
			node.innerHTML = data
		})
	}

	let newPollFormName = $state("")
	let newPollFormOptionA = $state("")
	let newPollFormOptionB = $state("")
</script>

<main>
	<div class="padding"></div>

	{#if worldState.type == "host"}
		<div class="host-persistent-info-card">
			<PollBubble>
				<GenericSection>
					<p class="before-room-code">Room Code</p>
					<h1 class="room-code">{worldState.hostingRoomCode}</h1>
					<div class="qr-code" use:qrCode={worldState.hostingRoomCode}></div>
				</GenericSection>
			</PollBubble>
		</div>
		{#each worldState.polls as poll (poll.id)}
			<PollBubble>
				<PollHeader poll={poll}>
					{#if !poll.revealed}
						<div class="reveal-poll-line">
							<button class="reveal-poll-button" onclick={()=>{
								communication.send({type:"reveal-poll", id: poll.id})
							}}>
								Reveal this poll!
							</button>
						</div>
					{/if}
				</PollHeader>
				<PollDivider/>
				<PollPredictionSegment poll={poll} />
				<PollDivider/>
				<GenericSection>
					<button class="delete-button" onclick={()=>{
						communication.send({type:"delete-poll", id: poll.id})
					}}>Delete This Poll</button>
				</GenericSection>
			</PollBubble>
		{/each}
		<PollBubble>
			<GenericSection>
				<div class="add-background"></div>
				<h1>Add a Poll</h1>
				<div class="add-poll-grid">
					<label>
						<div class="add-poll-grid-letter">?</div>
						<p>Question</p>
						<input type="text" bind:value={newPollFormName}>
					</label>
					<label>
						<div class="add-poll-grid-letter">A</div>
						<p>Option A</p>
						<input type="text" bind:value={newPollFormOptionA}>
					</label>
					<label>
						<div class="add-poll-grid-letter">B</div>
						<p>Option B</p>
						<input type="text" bind:value={newPollFormOptionB}>
					</label>
					<button class="add-poll-button" disabled={!newPollFormName || !newPollFormOptionA || !newPollFormOptionB} onclick={()=>{
						communication.send({type: "create-poll", question: newPollFormName, description:"", optionA: newPollFormOptionA, optionB: newPollFormOptionB})
						newPollFormName = ""
						newPollFormOptionA = ""
						newPollFormOptionB = ""
					}}>Create Poll</button>
				</div>
			</GenericSection>
		</PollBubble>
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
				<h1>An error occurred</h1>
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

	.host-persistent-info-card {
		position: fixed;
		top: 20px;
		left: 20px;
		z-index: 1;
	}
	.before-room-code {
		margin: 0px;
		text-align: left;
	}
	.room-code {
		font-size: 2.3em;
	}

	.add-poll-grid {
		display: grid;
		/* max-width: 450px; */
		margin: 10px auto;
		gap: 10px;
	}
	.add-poll-grid-letter {
		font-size: 2em;
		grid-area: letter;
		background-color: rgba(208, 78, 255, 0.6);
		color: white;
		font-weight: bold;
		width: 1em;
		height: 1em;
		padding: 5px;
		line-height: 1;
		border-radius: 5px;
		display: flex;
		justify-content: center;
		align-items: center;
	}
	.add-poll-grid label {
		display: grid;
		grid-template-areas: 'title title'
							 'letter input';

		grid-template-columns: min-content 1fr;
		grid-template-rows: min-content 1fr;

		gap: 5px;
	}
	.add-poll-grid label p {
		grid-area: title;
		margin: 0px;
		text-align: center;
		font-size: 1.1em;
	}
	.add-poll-grid label input {
		grid-area: input;
	}


	input[type="text"] {
		border: none;
		background-color: rgba(208, 78, 255, 0.07);
		border-bottom: 3px solid rgb(208, 78, 255);
		font: inherit;
		font-size: 1.2em;
		padding: 0px 10px;
		outline-color: rgb(208, 78, 255);
	}


	.padding {
		height: 100px;
	}

	button.add-poll-button {
		margin-top: 8px;
		justify-self: end;
		transform: rotate(-4deg);

		background-color: rgb(208, 78, 255);
		border: none;
		font: inherit;

		padding: 10px;
		padding-right: 30px;
		color: white;
		font-weight: bold;

		clip-path: polygon(0 0, calc(100% - 10px) 0, 100% 50%, calc(100% - 10px) 100%, 0 100%);

		transition: background-color 0.3s ease, filter 0.3s ease;
	}
	button.add-poll-button:focus-visible, button.add-poll-button:hover {
		background-color: rgb(164, 61, 201);
	}
	button.add-poll-button:active {
		background-color: rgb(89, 32, 110);
	}
	button.add-poll-button:disabled {
		background-color: rgb(208, 78, 255) !important;
		filter: saturate(0);
	}


	button.delete-button {
		background-color: rgb(255, 78, 93);
		border: none;
		font: inherit;

		padding: 5px;
		/* padding-right: 20px; */
		color: white;
		font-weight: bold;

		/* clip-path: polygon(0 0, 100% 0, calc(100% - 10px) 50%, 100% 100%, 0 100%); */

		transition: background-color 0.3s ease, filter 0.3s ease;
	}
	button.delete-button:focus-visible, button.delete-button:hover {
		background-color: rgb(215, 64, 76);
	}
	button.delete-button:active {
		background-color: rgb(145, 44, 53);
	}


	.reveal-poll-line {
		display: flex;
		align-items: center;
		justify-content: center;
	}

	button.reveal-poll-button {
		margin-top: 20px;

		background-color: rgb(208, 78, 255);
		border: none;
		font: inherit;

		padding: 10px;
		padding-left: 20px;
		color: white;
		font-weight: bold;

		clip-path: polygon(0 0, 100% 0, 100% 100%, 0 100%, 10px 50%);

		transition: background-color 0.3s ease, filter 0.3s ease;
	}
	button.reveal-poll-button:focus-visible, button.reveal-poll-button:hover {
		background-color: rgb(164, 61, 201);
	}
	button.reveal-poll-button:active {
		background-color: rgb(89, 32, 110);
	}
	button.reveal-poll-button:disabled {
		background-color: rgb(208, 78, 255) !important;
		filter: saturate(0);
	}
</style>
