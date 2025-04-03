<script lang="ts">
    import type { Communication } from "$lib/Communication.svelte";
    import type { Poll } from "$lib/types";
    import type { Snippet } from "svelte";
    import SvgPenCircle from "./SvgPenCircle.svelte";


	const {children, poll, communication}: {
		children?: Snippet,
		poll: Poll,
		communication: Communication
	} = $props()

</script>

<div class="card-holder">
	<div class="card">
		<div class="header">
			<h1>Vote!</h1>
		</div>
		<div class="card-body">
			<div class="form-segment">
				<h2>{poll.question}</h2>
				<div class="options">
					<SvgPenCircle show={true} size={50} color="rgb(208, 78, 255)" />
					<SvgPenCircle show={true} size={50} color="rgb(208, 78, 255)" />
					<SvgPenCircle show={true} size={50} color="rgb(208, 78, 255)" />
					<SvgPenCircle show={true} size={50} color="rgb(208, 78, 255)" />
				</div>
			</div>
		</div>
		<button>Submit</button>
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
	}
</style>