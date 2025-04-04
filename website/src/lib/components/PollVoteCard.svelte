<script lang="ts">
    import type { Communication } from "$lib/Communication.svelte";
    import type { Poll } from "$lib/types";
    import type { Snippet } from "svelte";
    import SvgPenCircle from "./SvgPenCircle.svelte";
    import { scale } from "svelte/transition";


	const {children, poll, communication}: {
		children?: Snippet,
		poll: Poll,
		communication: Communication
	} = $props()


	let vote = $state("")
	let prediction = $state("")

	const rotationA = $state(Math.random() * 2 - (2/2))
</script>

<div class="card-holder" style:transform={`rotate(${rotationA}deg)`}
	transition:scale
>
	<div class="card">
		<div class="header">
			<h1>Vote!</h1>
		</div>
		<div class="card-body">
			<div class="form-segment">
				<h2><span class="header-flavor">1. </span> {poll.question}</h2>
				<div class="options">
					<label class="option a">
						<input type="radio" value="a" bind:group={vote}>
						<p>A</p>
						<div class="pen">
							<SvgPenCircle show={vote == "a"} size={50} color="hsl(324, 100%, 50%)" />
						</div>
					</label>
					<label class="option b">
						<input type="radio" value="b" bind:group={vote}>
						<p>B</p>
						<div class="pen">
							<SvgPenCircle show={vote == "b"} size={50} color="hsl(147, 100%, 39%)" />
						</div>
					</label>
				</div>
				<p class="filler-center-text">(Circle ONE to vote)</p>
			</div>
			<div class="form-segment">
				<h2><span class="header-flavor">2. </span> Which do you predict will get the most votes?</h2>
				<div class="options">
					<label class="option a">
						<input type="radio" value="a" bind:group={prediction}>
						<p>A</p>
						<div class="pen">
							<SvgPenCircle show={prediction == "a"} size={50} color="hsl(324, 100%, 50%)" />
						</div>
					</label>
					<label class="option b">
						<input type="radio" value="b" bind:group={prediction}>
						<p>B</p>
						<div class="pen">
							<SvgPenCircle show={prediction == "b"} size={50} color="hsl(147, 100%, 39%)" />
						</div>
					</label>
				</div>
				<p class="filler-center-text">(Circle ONE to predict)</p>
			</div>

			<div class="button-wrapper">
				<button disabled={!vote || !prediction} onclick={()=>{
					communication.send({type: "vote", for: poll.id, votedA: vote=="a", predictedA: prediction=="a"})
				}} >Submit</button>
			</div>
		</div>
	</div>	
</div>


<style>
	p, h1 {
		margin: 0;
	}
	.card-holder {
		display: flex;
		justify-content: center;
		margin: 10px 0px;
	}
	.card {
		flex-grow: 1;
		max-width: 23em;
		margin: 0px 10px;
		background-color: white;
		border: 3px solid rgb(208, 78, 255);
		border-radius: 10px;

		display: grid;
	}

	.header {
		text-align: center;
		color: white;
		background-color: rgb(208, 78, 255);
		padding: 0.5em 10px;

		margin: -0.4px;
		border-radius: 4px 4px 0px 0px;
	}
	.header-flavor {
		opacity: 0.5;
	}

	input[type="radio"] {
		display: none;
	}

	.card-body {
		border: 3px dashed rgb(208, 78, 255);
		border-top: none;
		border-bottom: none;
		padding: 0px 10px;
		margin: 0px 3px;
	}

	.form-segment {

	}
	.form-segment h2 {
		margin: 0px;
		margin-top: 10px;
		font-size: 1.2em;
	}
	.form-segment .options {
		display: grid;
		grid-template-columns: 1fr 1fr;
		gap: 10px;
	}
	.form-segment .option {
		position: relative;
		display: flex;
		justify-content: center;
		align-items: center;
		background-color: rgba(232, 168, 255, 0.23);

		transition: background-color 0.3s ease;
	}
	.form-segment .option:hover, .form-segment .option:focus-visible {
		background-color: rgba(232, 168, 255, 0.371);
	}
	.form-segment .option:active {
		background-color: rgba(232, 168, 255, 0.5);
	}
	.form-segment .option p {
		font-size: 1.3em;
		font-weight: bold;
		padding: 10px;
		padding: 10px;
	}
	.form-segment .option .pen {
		position: absolute;
		top: 55%;
		left: 50%;
		transform: translate(-50%, -50%);
	}


	.filler-center-text {
		text-align: center;
		font-size: 0.7em;
		opacity: 0.5;
		margin-top: 1px;
		margin-bottom: 10px;
	}

	.button-wrapper {
		display: flex;
		justify-content: center;
		margin: 10px;
	}
	button {
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
	button:focus-visible, button:hover {
		background-color: rgb(164, 61, 201);
	}
	button:active {
		background-color: rgb(89, 32, 110);
	}
	button:disabled {
		background-color: rgb(208, 78, 255) !important;
		filter: saturate(0);
	}
</style>