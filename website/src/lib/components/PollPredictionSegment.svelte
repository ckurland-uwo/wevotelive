<script lang="ts">
    import type { Poll } from "$lib/types";
    import type { Snippet } from "svelte";

	const {children, poll}: {
		children?: Snippet,
		poll: Poll
	} = $props()

	const rotationA = $state(Math.random() * 5 - (5/2))
	const rotationB = $state(Math.random() * 5 - (5/2))

	const predictionPercentA = $derived((poll.predictionsA/(poll.predictionsA + poll.predictionsB))*100)
	const predictionPercentB = $derived((poll.predictionsB/(poll.predictionsA + poll.predictionsB))*100)
	const predictionPercentATruncated = $derived(predictionPercentA.toFixed(1))
	const predictionPercentBTruncated = $derived(predictionPercentB.toFixed(1))
</script>

<section class:revealed={poll.revealed} style={`--predictionsA: ${predictionPercentA}%; --predictionsB: ${predictionPercentB}%`}>
	<div class="background">
		<div class="a"></div>
		<div class="center"></div>
		<div class="b"></div>
	</div>
	<div class="content">
		<div class="predictions">
			<h1>Predictions</h1>
			<div class="options-display">
				<div class="option-card a" style:transform={`rotate(${rotationA}deg)`} class:winner={poll.revealed && poll.predictionsA >= poll.predictionsB}>
					<p class="option-number">A</p>
					<p class="option">{poll.predictionsA}</p>
					{#if !isNaN(predictionPercentA)}
						<div class="results-segment">
							<p class="percentage">{predictionPercentATruncated}%</p>
						</div>
					{/if}
				</div>
				<p class="vs">to</p>
				<div class="option-card b" style:transform={`rotate(${rotationB}deg)`} class:winner={poll.revealed && poll.predictionsA <= poll.predictionsB}>
					<p class="option-number">B</p>
					<p class="option">{poll.predictionsB}</p>
					{#if !isNaN(predictionPercentB)}
						<div class="results-segment">
							<p class="percentage">{predictionPercentBTruncated}%</p>
						</div>
					{/if}
				</div>
			</div>
		</div>
		{#if children}
			{@render children()}
		{/if}
	</div>
</section>

<style>
	.content {
		z-index: 1;
	}
	section {
		position: relative;
		padding: 10px 20px;
	}
	.predictions {
		text-align: center;
		color: white;
	}
	h1 {
		font-size: 1.5em;
		margin: 0px;
		margin-bottom: 10px;
		margin-top: 5px;
	}
	.options-display { 
		display: grid;
		font-size: 1.3em;
		justify-items: center;

		grid-template-columns: auto auto auto;
		justify-content: center;
    	align-items: center;
	}
	.options-display p {
		margin: 0px;
	}
	.vs {
		font-size: 0.8em;
		opacity: 0.7;
		padding: 10px;
	}
	.option-card {
		display: grid;
		grid-template-columns: min-content 1fr min-content;
		border: 3px solid black;
		border-radius: 7px;
		align-items: center;
	}
	.option-card .option-number {
		color: white;
		font-weight: bold;
		width: 1em;
		height: 1em;
		padding: 5px;
		margin: 3px;
		line-height: 1;
		border-radius: 2px;
		align-self: stretch;
	}
	.option-card .option {
		text-align: left;
		margin: 5px 10px;
		font-weight: 600;
	}
	.option-card.a {
		color: hsl(324, 100%, 50%);
		background-color: hsla(0, 0%, 100%);
		border-color: hsla(324, 100%, 50%, 0.7);
	}
	.option-card.a .option-number {
		background-color: hsl(324, 100%, 50%);
	}
	.option-card.b {
		color: hsl(147, 100%, 39%);
		background-color: hsla(0, 0%, 100%);
		border-color: hsla(147, 100%, 39%, 0.7);
	}
	.option-card.b .option-number {
		background-color: hsl(147, 100%, 39%);
	}
	.option-card.a .results-segment {
		background-color: hsl(324, 100%, 50%);
	}
	.option-card.b .results-segment {
		background-color: hsl(147, 100%, 39%);
	}
	.option-card.winner {
		scale: 1.15;
		margin: 10px 15px;
		z-index: 1;
	}

	.results-segment {
		align-self: flex-start;
		display: grid;
		grid-template-areas: 
		'percent divider text'
		'votes divider text';
		margin: 3px;
		border-radius: 2px;
		align-items: center;
		line-height: 1;
		padding: 5px;
		color: white;
	}
	.results-segment .votes {
		grid-area: votes;
		font-size: 0.7em;
	}
	.results-segment .percentage {
		grid-area: percent;
		font-size: 1.1em;
	}
	.results-segment .winner {
		grid-area: text;
		font-size: 0.8em;
	}
	.results-segment .divider {
		grid-area: divider;
		align-self: stretch;
		width: 1px;
		background-color: white;
		margin: 0px 5px;
	}

	.background {
		z-index: -1;
		position: absolute;
		inset: 0;

		display: grid;
		grid-template-columns: 0% 1fr 0%;
		grid-template-rows: 1fr;

		transition: grid-template-columns 0.3s ease;

		filter: blur(10px);
	}
	.background .a {
		background-color: hsl(312, 74%, 26%);
	}
	.background .b {
		background-color: hsl(155, 67%, 20%);
	}
	.background .center {
		background-color: rgb(27, 9, 35, 0.6);
	}
	@property --predictionsA {
		syntax: '<percentage>';
		inherits: true;
		initial-value: 0%;
	}

	@property --predictionsB {
		syntax: '<percentage>';
		inherits: true;
		initial-value: 0%;
	}

	section.revealed .background {
		grid-template-columns: var(--predictionsA) 1fr var(--predictionsB);
	}


	@media (max-width: 425px) {
		.option-card {
			grid-template-columns: auto 1fr;
		}
		.results-segment {
			grid-column: span 2;
			grid-template-columns: 1fr;
		}
	}
</style>