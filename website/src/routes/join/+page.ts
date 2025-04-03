import type { PageLoad } from './$types';

export const load = (async ({url}) => {
    return {
        roomCode: url.searchParams.get('room')
    };
}) satisfies PageLoad;