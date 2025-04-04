<script lang="ts">
    import GenericSection from '$lib/components/GenericSection.svelte';
    import PollBubble from '$lib/components/PollBubble.svelte';
	import '$lib/css/global.css'
	import title from '$lib/assets/title.svg'
    import { fade } from 'svelte/transition';


	let roomCode = $state("")

</script>

<main>
	<img src={title} alt="WeVote Live" height="100px" style="margin-bottom: 30px;" transition:fade={{duration: 300}}>
	<PollBubble>
		<GenericSection>
			<p>Get started by hosting or joining a room.</p>
			<div class="options-grid">
				<div class="host">
					<a href="/host">
						<button class="reveal-poll-button">Host</button>
					</a>
				</div>
				<div class="join">
					<input type="text" maxlength="4" placeholder="Room Code" bind:value={roomCode}>
					<a style="display: grid" href={roomCode.length == 4? `https://wevotelive.azurestudios.ca/join/?room=${roomCode}` : ""}>
						<button class="add-poll-button" disabled={roomCode.length != 4}>Join</button>
					</a>
				</div>
			</div>
			<p class="codes-info">Room codes are case-sensitive.</p>
		</GenericSection>
	</PollBubble>
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
	}
	p {
		text-align: center;
	}

	input[type="text"] {
		max-width: 4em;
		min-width: 0;
		border: none;
		background-color: rgba(208, 78, 255, 0.07);
		border-bottom: 3px solid rgb(208, 78, 255);
		font: inherit;
		font-size: 2em;
		padding: 0px 10px;
		outline-color: rgb(208, 78, 255);
	}
	input[type="text"]::placeholder {
		font-size: 0.7em;
	}

	button.reveal-poll-button {
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

	button.add-poll-button {

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


	.options-grid {
		display: grid;
		grid-template-columns: 1fr;
		gap: 10px;
		justify-items: center;
		align-items: center;
	}
	.join {
		display: grid;
		grid-template-columns: 1fr min-content;
	}


	a {
		text-decoration: none;
	}




	.codes-info {
		font-size: 0.8em;
		margin: 0px;
		margin-top: 5px;
		opacity: 0.7;
	}
</style>

