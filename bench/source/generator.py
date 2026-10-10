import random


GENERATOR_PATTERNS = (
    "scan", "cyclic_working_set", "just_over_cache", "hot_cold", "zipf", "strong_zipf",
    "random", "burst", "switching_working_sets", "returning_working_sets",
    "alternating_regions", "drifting_popularity", "flash_crowd", "recent_history_reuse",
    "delayed_history_reuse", "periodic_hot_set", "three_frequency_tiers", "local_walk",
    "scan_with_reuse", "two_phase", "hot_noise", "repeated_scan", "mixed_scan_random",
    "working_set_growth", "working_set_shrink", "rotating_hot_sets", "random_bursts",
    "clustered_random", "looping_working_set",
)


class Generator:
    def __init__(self, seed, data_len, key_count):
        if data_len <= 0:
            raise ValueError("data_len must be > 0")
        if key_count <= 0:
            raise ValueError("key_count must be > 0")
        self.data_len = data_len
        self.key_count = key_count
        self.rng = random.Random(seed)

    def _check_cache_size(self, cache_size):
        if cache_size <= 0:
            raise ValueError("cache_size must be > 0")

    def _random_key(self):
        return self.rng.randint(1, self.key_count)

    def _weighted_random(self, weights):
        if len(weights) != self.key_count:
            raise ValueError("weights must have key_count elements")
        total = sum(weights)
        if total <= 0:
            raise ValueError("sum of weights must be > 0")

        cumulative = []
        acc = 0.0
        for weight in weights:
            if weight < 0:
                raise ValueError("weights must be non-negative")
            acc += weight / total
            cumulative.append(acc)

        result = []
        for _ in range(self.data_len):
            x = self.rng.random()
            lo, hi = 0, self.key_count - 1
            while lo < hi:
                mid = (lo + hi) // 2
                if cumulative[mid] <= x:
                    lo = mid + 1
                else:
                    hi = mid
            result.append(lo + 1)
        return result

    def _random_set(self, size):
        size = min(size, self.key_count)
        return self.rng.sample(range(1, self.key_count + 1), size)

    def _random_from_set(self, keys, count):
        return [self.rng.choice(keys) for _ in range(count)]

    def scan(self, cache_size):
        self._check_cache_size(cache_size)
        return [(i % self.key_count) + 1 for i in range(self.data_len)]

    def cyclic_working_set(self, cache_size):
        self._check_cache_size(cache_size)
        ws_size = min(self.key_count, cache_size)
        return [(i % ws_size) + 1 for i in range(self.data_len)]

    def just_over_cache(self, cache_size):
        self._check_cache_size(cache_size)
        ws_size = min(self.key_count, cache_size + 1)
        return [(i % ws_size) + 1 for i in range(self.data_len)]

    def hot_cold(self, cache_size):
        self._check_cache_size(cache_size)
        hot_size = max(1, min(self.key_count, cache_size // 2, self.key_count // 4))
        hot = list(range(1, hot_size + 1))
        cold = list(range(hot_size + 1, self.key_count + 1))
        return [
            self.rng.choice(hot if not cold or self.rng.random() < 0.8 else cold)
            for _ in range(self.data_len)
        ]

    def zipf(self, cache_size):
        self._check_cache_size(cache_size)
        return self._weighted_random([1.0 / i ** 1.2 for i in range(1, self.key_count + 1)])

    def strong_zipf(self, cache_size):
        self._check_cache_size(cache_size)
        return self._weighted_random([1.0 / i ** 2.0 for i in range(1, self.key_count + 1)])

    def random(self, cache_size):
        self._check_cache_size(cache_size)
        return [self._random_key() for _ in range(self.data_len)]

    def burst(self, cache_size):
        self._check_cache_size(cache_size)
        result = []
        while len(result) < self.data_len:
            low = max(1, min(cache_size // 2, self.key_count))
            high = max(low, min(cache_size * 2, self.key_count))
            ws_size = self.rng.randint(low, high)
            working_set = self._random_set(ws_size)
            burst_len = self.rng.randint(max(1, cache_size), max(1, cache_size * 4))
            result.extend(self._random_from_set(working_set, min(
                burst_len, self.data_len - len(result)
            )))
        return result

    def switching_working_sets(self, cache_size):
        self._check_cache_size(cache_size)
        ws_size = min(self.key_count, cache_size)
        sets = [self._random_set(ws_size) for _ in range(4)]
        result = []
        while len(result) < self.data_len:
            for working_set in sets:
                count = min(self.rng.randint(cache_size, cache_size * 4),
                            self.data_len - len(result))
                result.extend(self._random_from_set(working_set, count))
        return result

    def returning_working_sets(self, cache_size):
        self._check_cache_size(cache_size)
        ws_size = max(1, min(self.key_count, max(1, self.key_count // 3)))
        sets = []
        for start in range(1, self.key_count + 1, ws_size):
            end = min(self.key_count, start + ws_size - 1)
            sets.append(list(range(start, end + 1)))
        if len(sets) > 3:
            sets = [sets[0], sets[len(sets) // 2], sets[-1]]
        result = []
        while len(result) < self.data_len:
            for working_set in sets:
                count = min(ws_size * 2, self.data_len - len(result))
                result.extend(self._random_from_set(working_set, count))
        return result

    def alternating_regions(self, cache_size):
        self._check_cache_size(cache_size)
        size = max(1, min(cache_size, max(1, self.key_count // 4)))
        left = list(range(1, size + 1))
        right_start = self.key_count - size + 1
        right = list(range(right_start, self.key_count + 1))
        if left == right:
            return self.returning_working_sets(cache_size)

        result = []
        while len(result) < self.data_len:
            for working_set in (left, right):
                count = min(size, self.data_len - len(result))
                result.extend(self._random_from_set(working_set, count))
        return result

    def drifting_popularity(self, cache_size):
        self._check_cache_size(cache_size)
        hot_size = max(1, min(cache_size, self.key_count))
        step = max(1, self.data_len // 20)
        center = self.rng.randrange(self.key_count)
        result = []
        for i in range(self.data_len):
            if i and i % step == 0:
                center = (center + max(1, hot_size // 2)) % self.key_count
            offset = self.rng.randrange(hot_size)
            result.append(((center + offset) % self.key_count) + 1)
        return result

    def flash_crowd(self, cache_size):
        self._check_cache_size(cache_size)
        hot_size = max(1, min(cache_size, self.key_count))
        flash_set = self._random_set(hot_size)
        result = []
        start = self.data_len * 0.4
        end = self.data_len * 0.6
        for i in range(self.data_len):
            result.append(self.rng.choice(flash_set) if start <= i < end else self._random_key())
        return result

    def recent_history_reuse(self, cache_size):
        self._check_cache_size(cache_size)
        history_size = max(1, cache_size * 2)
        result = []
        history = []
        for _ in range(self.data_len):
            if history and self.rng.random() < 0.8:
                key = self.rng.choice(history)
            else:
                key = self._random_key()
            result.append(key)
            history.append(key)
            if len(history) > history_size:
                history.pop(0)
        return result

    def delayed_history_reuse(self, cache_size):
        self._check_cache_size(cache_size)
        delay = max(1, min(self.data_len // 3, cache_size * 2))
        result = []
        for i in range(self.data_len):
            if i >= delay and self.rng.random() < 0.6:
                result.append(result[i - delay])
            else:
                result.append(self._random_key())
        return result

    def periodic_hot_set(self, cache_size):
        self._check_cache_size(cache_size)
        hot_size = max(1, min(cache_size, self.key_count))
        hot = self._random_set(hot_size)
        period = max(hot_size * 4, cache_size * 4)
        result = []
        for i in range(self.data_len):
            result.append(self.rng.choice(hot) if i % period < hot_size else self._random_key())
        return result

    def three_frequency_tiers(self, cache_size):
        self._check_cache_size(cache_size)
        if self.key_count == 1:
            return [1] * self.data_len
        hot_end = max(1, self.key_count // 10)
        warm_end = min(self.key_count, max(hot_end + 1, self.key_count // 2))
        result = []
        for _ in range(self.data_len):
            r = self.rng.random()
            if r < 0.60:
                result.append(self.rng.randint(1, hot_end))
            elif r < 0.90 and hot_end < warm_end:
                result.append(self.rng.randint(hot_end + 1, warm_end))
            else:
                result.append(self.rng.randint(warm_end + 1, self.key_count)
                              if warm_end < self.key_count else self.rng.randint(1, hot_end))
        return result

    def local_walk(self, cache_size):
        self._check_cache_size(cache_size)
        current = self.rng.randrange(self.key_count)
        result = []
        for _ in range(self.data_len):
            result.append(current + 1)
            current = (current + self.rng.choice([-3, -2, -1, 0, 1, 2, 3])) % self.key_count
        return result

    def scan_with_reuse(self, cache_size):
        self._check_cache_size(cache_size)
        reuse_limit = min(self.key_count, cache_size)
        result = []
        for i in range(self.data_len):
            if self.rng.random() < 0.2:
                result.append(self.rng.randint(1, reuse_limit))
            else:
                result.append((i % self.key_count) + 1)
        return result

    def two_phase(self, cache_size):
        self._check_cache_size(cache_size)
        split = self.data_len // 2
        ws_size = max(1, min(cache_size, max(1, self.key_count // 3)))
        first = list(range(1, ws_size + 1))
        second_start = self.key_count - ws_size + 1
        second = list(range(second_start, self.key_count + 1))
        if first == second:
            second = first
        result = self._random_from_set(first, split)
        result.extend(self._random_from_set(second, self.data_len - split))
        return result

    def hot_noise(self, cache_size):
        self._check_cache_size(cache_size)
        hot_size = max(1, min(cache_size, self.key_count))
        hot = self._random_set(hot_size)
        return [
            self.rng.choice(hot) if self.rng.random() < 0.7 else self._random_key()
            for _ in range(self.data_len)
        ]

    def repeated_scan(self, cache_size):
        self._check_cache_size(cache_size)
        scan_size = min(self.key_count, max(1, cache_size))
        scan = list(range(1, scan_size + 1))
        result = []
        while len(result) < self.data_len:
            result.extend(scan[:self.data_len - len(result)])
        return result

    def mixed_scan_random(self, cache_size):
        self._check_cache_size(cache_size)
        result = []
        for i in range(self.data_len):
            result.append((i % self.key_count) + 1 if self.rng.random() < 0.7 else self._random_key())
        return result

    def working_set_growth(self, cache_size):
        self._check_cache_size(cache_size)
        sizes = [max(1, cache_size // 2), cache_size, cache_size * 2]
        base, extra = divmod(self.data_len, len(sizes))
        result = []
        for i, size in enumerate(sizes):
            keys = self._random_set(min(size, self.key_count))
            result.extend(self._random_from_set(keys, base + (i < extra)))
        return result

    def working_set_shrink(self, cache_size):
        self._check_cache_size(cache_size)
        sizes = [cache_size * 2, cache_size, max(1, cache_size // 2)]
        base, extra = divmod(self.data_len, len(sizes))
        result = []
        for i, size in enumerate(sizes):
            keys = self._random_set(min(size, self.key_count))
            result.extend(self._random_from_set(keys, base + (i < extra)))
        return result

    def rotating_hot_sets(self, cache_size):
        self._check_cache_size(cache_size)
        hot_size = max(1, min(cache_size, self.key_count))
        sets = []
        for start in range(1, self.key_count + 1, hot_size):
            keys = list(range(start, min(self.key_count, start + hot_size - 1) + 1))
            if keys not in sets:
                sets.append(keys)
        result = []
        while len(result) < self.data_len:
            for keys in sets:
                count = min(max(1, cache_size * 2), self.data_len - len(result))
                result.extend(self._random_from_set(keys, count))
        return result

    def random_bursts(self, cache_size):
        self._check_cache_size(cache_size)
        result = []
        while len(result) < self.data_len:
            key = self._random_key()
            count = self.rng.randint(1, max(1, cache_size * 2))
            result.extend([key] * min(count, self.data_len - len(result)))
        return result

    def clustered_random(self, cache_size):
        self._check_cache_size(cache_size)
        cluster_size = max(1, min(self.key_count, cache_size * 2))
        result = []
        while len(result) < self.data_len:
            start = self.rng.randint(1, self.key_count - cluster_size + 1)
            cluster = list(range(start, start + cluster_size))
            count = min(self.rng.randint(1, cache_size * 2), self.data_len - len(result))
            result.extend(self._random_from_set(cluster, count))
        return result

    def looping_working_set(self, cache_size):
        self._check_cache_size(cache_size)
        ws_size = min(self.key_count, cache_size)
        working_set = self._random_set(ws_size)
        result = []
        while len(result) < self.data_len:
            order = working_set.copy()
            self.rng.shuffle(order)
            result.extend(order[:self.data_len - len(result)])
        return result

    def generate(self, pattern, cache_size):
        if pattern not in GENERATOR_PATTERNS:
            raise ValueError(f"Unknown pattern: {pattern}")
        return getattr(self, pattern)(cache_size)

    def generate_all(self, cache_size):
        return {pattern: getattr(self, pattern)(cache_size) for pattern in GENERATOR_PATTERNS}
