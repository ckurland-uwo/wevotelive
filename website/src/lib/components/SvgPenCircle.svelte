<script lang="ts">

	let {show, size, color}: {
		show: boolean,
		size: number,
		color: string
	} = $props()

	const p = [18, 25, 73, -22, 125, 30, 78, 57, 20, 51, 19, 46, 18, 25, 40, 14]
	const v = [10, 10, 10,  10,  10, 10, 10,  0,  0,  0,  0,  0,  0,  0,  0,  5]
	let n: number[] = $state([0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0])

	function randomRange(min: number, max: number): number {
		return Math.random() * (max - min) + min
	}
	function randomVariance(amount: number): number {
		return randomRange(-amount, amount)
	}

	$effect(()=>{
		if (show) return		
		for (let i = 0; i < p.length; i++) {
			n[i] = p[i] + randomVariance(v[i])
		}
	})

	for (let i = 0; i < p.length; i++) {
		n[i] = p[i] + randomVariance(v[i])
	}
</script>

{#if show}
	<svg width={size} height={size} viewBox="0 0 100 100" style="overflow: visible;">
		<path 
			d={`M ${n[0]} ${n[1]} C ${n[2]} ${n[3]} ${n[4]} ${n[5]} ${n[6]} ${n[7]} S ${n[8]} ${n[9]} ${n[10]} ${n[11]} S ${n[12]} ${n[13]} ${n[14]} ${n[15]}`} 
			stroke={color}
			stroke-width="5"
			stroke-linecap="round"
			fill="none"
			transform="translate(-20 0) scale(1.3)"
			pathLength="100"
			stroke-dasharray="100"
		></path>
	</svg>
{/if}

<style>
	@keyframes draw {
		from {
			stroke-dashoffset: 99.9;
		}
		to {
			stroke-dashoffset: 0;
		}
	}
	path {
		animation: draw 0.5s cubic-bezier(.38,0,0,1);
	}
</style>