{
    "files": [{
        "aql": 
        {
            "items.find": 
            {
                "repo": "hpccpl-docker-local",
                "@build.number": {"$match":"*-rc*"},
                "created": {"$before": "1mo"}
            }
        }
    }]
}