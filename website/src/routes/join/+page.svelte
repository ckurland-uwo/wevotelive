<script lang="ts">
    import { Communication } from '$lib/Communication.svelte';
	import Card from '$lib/components/Card.svelte';
    import PollBubble from '$lib/components/PollBubble.svelte';
    import PollDivider from '$lib/components/PollDivider.svelte';
    import PollHeader from '$lib/components/PollHeader.svelte';
    import PollPredictionSegment from '$lib/components/PollPredictionSegment.svelte';
    import PollVoteCard from '$lib/components/PollVoteCard.svelte';
    import PollYouSection from '$lib/components/PollYouSection.svelte';
import '$lib/css/global.css'
	import type { PageData } from './$types';

	let { data }: { data: PageData } = $props();
</script>

<main>
	<!-- <Card>
		<h1>Hello!</h1>
		<p>This is a card.</p>
	</Card> -->
	<PollBubble>
		<PollHeader poll={{
			id: "...",
			question: "Which is better?",
			description: "description",
			optionA: "iCloud+",
			optionB: "Google Business Profile Manager",
			votesA: 0,
			votesB: 0,
			predictionsA: 0,
			predictionsB: 0,
			revealed: false

		}} />
		<PollVoteCard poll={{
			id: "...",
			question: "Which is better?",
			description: "description",
			optionA: "iCloud+",
			optionB: "Google Business Profile Manager",
			votesA: 0,
			votesB: 0,
			predictionsA: 0,
			predictionsB: 0,
			revealed: false

		}} communication={new Communication()}/>
		<PollDivider/>
		<PollPredictionSegment poll={{
			id: "...",
			question: "Which is better?",
			description: "description",
			optionA: "iCloud+",
			optionB: "Google Business Profile Manager",
			votesA: 0,
			votesB: 0,
			predictionsA: 0,
			predictionsB: 0,
			revealed: false
		}}
		/>
		<PollDivider/>
	</PollBubble>
	<PollBubble>
		<PollHeader poll={{
			id: "...",
			question: "Which is better?",
			description: "description",
			optionA: "iCloud+",
			optionB: "Google Business Profile Manager",
			votesA: 87434,
			votesB: 54239,
			predictionsA: 43278,
			predictionsB: 21436,
			revealed: true

		}} />
		<PollDivider/>
		<PollPredictionSegment poll={{
			id: "...",
			question: "Which is better?",
			description: "description",
			optionA: "iCloud+",
			optionB: "Google Business Profile Manager",
			votesA: 87434,
			votesB: 54239,
			predictionsA: 43278,
			predictionsB: 21436,
			revealed: true
		}}/>
		<PollDivider/>
		<PollYouSection poll={{
			id: "...",
			question: "Which is better?",
			description: "description",
			optionA: "iCloud+",
			optionB: "Google Business Profile Manager",
			votesA: 87434,
			votesB: 54239,
			predictionsA: 43278,
			predictionsB: 21436,
			revealed: true
		}} vote={{predictedA: true, votedA: false}}/>
	</PollBubble>
	<PollBubble>
		<PollHeader poll={{
			id: "...",
			question: "Which of the two would you like more as a gift?",
			description: "description",
			optionA: "80 kilograms of pure lead in a single large barrel",
			optionB: "286 rubber ducks hidden around your house, place of work, and other areas you frequent; each containing a small amount of gold",
			votesA: 23897,
			votesB: 67823,
			predictionsA: 14373,
			predictionsB: 51436,
			revealed: false
		}} />
		<PollDivider/>
		<PollPredictionSegment poll={{
			id: "...",
			question: "Which of the two would you like more as a gift?",
			description: "description",
			optionA: "80 kilograms of pure lead in a single large barrel",
			optionB: "286 rubber ducks hidden around your house, place of work, and other areas you frequent; each containing a small amount of gold",
			votesA: 23897,
			votesB: 67823,
			predictionsA: 59925,
			predictionsB: 23231,
			revealed: false
		}}/>
		<PollDivider/>
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

		gap: 10px;
	}
</style>
