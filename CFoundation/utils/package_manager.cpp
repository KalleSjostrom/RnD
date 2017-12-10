struct PackageInfo {
	IdString64 path;
	PatchedResourcePackagePtr package;
};

// Maps goid to index in instance_storage
// FULKOD(kalle): Either move away from the way we do hashmaps
DEPRECATED_HASH_ENTRY(IdPackageInfoEntry, uint64_t, PackageInfo);
DEPRECATED_HASH_MAP(PackageHashmap, IdPackageInfoEntry, 32);

static void _flush_package(PackageInfo &info) {
	uint64_t start = _Timer.ticks();
		_Application.complete(info.package);
	uint64_t stop = _Timer.ticks();
	double seconds = _Timer.ticks_to_seconds(stop - start);
	LOG_INFO("PackageManager", "Package %s flushed in %.3f s.", info.path.to_string(), (float)seconds);
}

struct PackageManager {
	PackageHashmap package_hashmap;

	void load_package(IdString64 path, bool flush) {
		Profile p("[PackageManager] load_package");
		LOG_INFO("ResourceHandling", "load_package (path=%s, flush=%d)", path.to_string(), flush);

		DEPRECATED_HASH_LOOKUP(entry, package_hashmap, path.id());
		PackageInfo &package_info = entry->value;

		if(has_loaded_package(path))
			return;

		if (entry->key == path.id()) { // We've been told to load this before
			if (flush) {
				Profile flush_p("flushing partially loaded package");
				_flush_package(package_info);
			}
		} else {
			StepProfile sp("Application.resource_package");
			PatchedResourcePackagePtr package = _Application.resource_package(path);

			entry->key = path.id();

			package_info.path = path;
			package_info.package = package;

			// Note: This puts the item in a queue and all calls after this to 'ResourcePackage.flush' will stall until this is done.
			sp.step("ResourcePackage.load");
			_Application.load(package);

			if (flush) {
				sp.step("ResourcePackage.flush");
				_flush_package(package_info);
			}
		}
	}

	bool has_loaded_package(IdString64 path) {
		DEPRECATED_HASH_LOOKUP(entry, package_hashmap, path.id());
		PackageInfo &package_info = entry->value;

		return (package_info.package != 0 && _Application.has_loaded(package_info.package));
	}
};
