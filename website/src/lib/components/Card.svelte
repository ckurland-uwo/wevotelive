<script lang="ts">
	const {children} = $props()
</script>

<div class="outer">
	<div class="inner">
		{@render children()}
	</div>
</div>

<style>
	.outer {
		border-radius: 10px;
		box-shadow: 
			0 0 3px hsla(0, 0%, 0%, 0.3),
			0px 0px 5px 0px rgba(255, 255, 255, 0.5) inset,
			-3px -4px 15px -4px hsla(261, 94%, 79%, 0.576) inset,
			5px 7px 10px -4px rgba(255, 255, 255, 0.9) inset,
			0px 0px 4px 1px hsla(261, 94%, 79%, 0.576) inset
		;
		padding: 5px;
		display: grid;
		grid-template-columns: 1fr;
		grid-template-rows: 1fr;

		backdrop-filter: blur(10px);

		background-color: #be9bff19;
	}
	.inner {
		border-radius: 5px;
		background-color: rgba(255, 255, 255, 0.5);
		box-shadow: 1px 1px 3px 1px hsla(0, 0%, 100%, 0.4);
		padding: 5px 10px;

		backdrop-filter: brightness(1.5);

		color: hsl(264, 48%, 61%);
	}
</style>