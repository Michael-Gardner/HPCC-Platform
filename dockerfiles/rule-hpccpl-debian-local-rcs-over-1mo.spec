{
    "files": [{
        "aql": 
        {
            "items.find": 
            {
                "repo": "hpccpl-debian-local",
                "path": "pool/LN",
                "name": {"$match": "*-rc*"},
                "created": {"$before": "1mo"}
            }
        }
    }]
}
