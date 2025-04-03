<script lang="ts">

	const {children} = $props()

</script>

<section>
	{@render children()}
</section>

<style>
	section {
		position: relative;
		padding: 10px 20px;
	}
</style>