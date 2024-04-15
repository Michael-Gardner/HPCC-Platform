{
    "files": [{
        "aql": 
        {
            "items.find": 
            {
                "repo": "hpccpl-docker-local",
                "type": "folder",
                "name": {"$match":"*8.6.24-rc1"},
                "created": {"$before": "1mo"}
            }
        }
    }]
}