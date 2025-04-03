<script lang="ts">
    import type { Snippet } from "svelte";


	const {children, onClick}: {
		children?: Snippet,
		onClick?: () => void
	} = $props()
</script>

<button onclick={onClick}>
	{@render children?.()}
</button>

<style>
	button {
		
	}
</style>