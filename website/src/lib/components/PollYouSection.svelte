<script lang="ts">
    import type { Communication } from "$lib/Communication.svelte";
    import type { Poll, UserVote } from "$lib/types";
    import type { Snippet } from "svelte";
    import PollButton from "./PollButton.svelte";
    import { expandVertically } from "$lib/Transitions";

	const {children, poll, vote}: {
		children?: Snippet,
		poll: Poll,
		vote: UserVote,
	} = $props()

	const aWon = $derived(poll.votesA >= poll.votesB)
	const bWon = $derived(poll.votesB >= poll.votesA)

	const rotationA = $state(Math.random() * 5 - (5/2))
	const rotationB = $state(Math.random() * 5 - (5/2))
</script>

<div class="content" transition:expandVertically={{duration: 300}} >
	<section class="you-section">
		<h1>You</h1>
		<div class="stuff-grid">
			<div class="statistic">
				<p class="label">Voted For</p>
				<div class="side-letter" class:a={vote.votedA} class:b={!vote.votedA}>
					<p>{vote.votedA ? "A" : "B"}</p>
					{#if (vote.votedA? aWon : bWon) && poll.revealed}
						<div class="correct-sticker" style:--additional-rotation={rotationA}>Winner!</div>
					{/if}
				</div>
			</div>
			<div class="statistic">
				<p class="label">Predicted</p>
				<div class="side-letter" class:a={vote.predictedA} class:b={!vote.predictedA}>
					<p>{vote.predictedA ? "A" : "B"}</p>
					{#if (vote.predictedA? aWon : bWon) && poll.revealed}
						<div class="correct-sticker" style:--additional-rotation={rotationB}>Correct!</div>
					{/if}
				</div>
			</div>
		</div>
	</section>
	{#if children}
		{@render children()}
	{/if}
</div>


<style>
	p {
		margin: 0;
	}
	.content {
		z-index: 1;
	}
	section {
		position: relative;
		padding: 10px 20px;
	}
	.you-section {
		text-align: center;
	}
	h1 {
		font-size: 1.5em;
		margin: 0px;
		margin-bottom: 10px;
		margin-top: 5px;
	}

	.stuff-grid {
		display: grid;
		grid-template-columns: 1fr 1fr;
	}
	.statistic {
		display: grid;
		justify-items: center;
		gap: 5px;
		margin-bottom: 20px;
	}
	.side-letter {
		position: relative;
		font-size: 2em;
		border-radius: 2px;
		color: white;
		font-weight: bold;
		width: 1em;
		height: 1em;
		padding: 5px;
		line-height: 1;
	}
	.side-letter.a {
		background-color: hsl(324, 100%, 50%);
	}
	.side-letter.b {
		background-color: hsl(147, 100%, 39%);
	}
	.correct-sticker {
		border-radius: 100px;
		font-weight: normal;
		font-size: 0.5em;
		background-color: rgb(243, 78, 255);
		position: absolute;
		padding: 5px 10px;
		bottom: -5px;
		right: 0px;
		transform: translate(50%, 50%) rotate(-12deg) rotate(calc(var(--additional-rotation) * 1deg));
	}
</style>