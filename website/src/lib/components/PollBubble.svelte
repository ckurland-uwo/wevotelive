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

		background-color: hsla(261, 100%, 89%, 0.2);

		display: grid;

		color: hsl(264, 51%, 19%);
	}
	.outer::after {
		content: '';
		position: absolute;
		inset: 0;

		border-radius: inherit;
		box-shadow: 
		0 0 3px hsla(0, 0%, 0%, 0.1),
			2px 4px 4px 0px rgba(255,255,255,0.8) inset,
			0px 0px 4px 0px hsla(261, 94%, 79%, 1) inset
		;

		/* border: 2px solid rgba(210, 186, 255, 0.468); */

		pointer-events: none;
	}
	.inner {
		border-radius: inherit;
		overflow:hidden
	}
</style>