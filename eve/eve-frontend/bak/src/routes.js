import Home from './routes/Home.svelte'
import Map from './routes/Map.svelte'
import Ws from './routes/Ws.svelte'

// Export the route definition object
export default {
    // Exact path
    '/': Home,
    '/map': Map,
    '/map/': Map,
    '/ws': Ws,
    '/ws/': Ws,
}
