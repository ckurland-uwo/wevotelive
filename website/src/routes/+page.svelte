<script lang="ts">
    import PollBubble from '$lib/components/PollBubble.svelte';
import '$lib/css/global.css'
</script>

<main>
	<PollBubble>
		<h1>Home Page</h1>
		<p>Get started by hosting or joining a room.</p>
		<a href="/host">Host</a>
		<a href="/join">Join</a>
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
</style>

