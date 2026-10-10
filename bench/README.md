# Our results

We've tested various data patterns using up to three levels of cache. First runs showed us that there is absolutely no need for more than three levels. Such configurations win rarely and more levels can't outperform at all.

We've tested two ratios of requests to keys counts. The first benchmark uses more keys than requests (see its [README.md](assets/more_keys_than_requests/README.md)), while the second uses the opposite ratio, with more requests than keys ([README.md](assets/more_requests_than_keys/README.md)).
