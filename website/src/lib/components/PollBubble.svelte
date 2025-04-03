<script lang="ts">
	const {children} = $props()
</script>


<article class="outer">
	<div class="inner">
		{@render children()}
	</div>
</article>

<style>
	.outer {
		position: relative;
		border-radius: 20px;

		backdrop-filter: blur(20px);

		/* background-color: hsla(261, 100%, 89%, 0.2); */

		display: grid;

		color: hsl(264, 51%, 19%);

		box-shadow: 0px 0px 4px 0px rgba(0, 0, 0, 0.113);
	}
	.outer::after {
		content: '';
		position: absolute;
		inset: 0;

		border-radius: inherit;

		border: 2px solid rgba(255, 255, 255, 0.767);

		pointer-events: none;
	}
	.inner {
		border-radius: inherit;
		overflow:hidden
	}
</style>