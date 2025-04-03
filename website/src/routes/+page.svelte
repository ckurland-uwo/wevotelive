<script lang="ts">
	import Card from '$lib/components/Card.svelte';
import '$lib/css/global.css'
</script>

<main>
	<Card>
		<h1>Home Page</h1>
		<p>Get started by hosting or joining a room.</p>
		<a href="/host">Host</a>
		<a href="/join">Join</a>
	</Card>
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

