"use client";

import React, { useState, useMemo } from "react";
import Link from "next/link";
import {
  COST_ITEMS,
  TOTAL_FACTORY_COST,
  TOTAL_FACTORY_COST_FORMATTED,
  PRICING_MODEL,
  MODULE_SUMMARIES,
  RAW_CSV,
  CostItem,
} from "./data";

export default function CostListClient() {
  const [searchQuery, setSearchQuery] = useState("");
  const [selectedModule, setSelectedModule] = useState<string>("All");
  const [sortBy, setSortBy] = useState<"default" | "cost-desc" | "cost-asc" | "name">("default");
  const [copiedCSV, setCopiedCSV] = useState(false);
  const [showRawCsv, setShowRawCsv] = useState(false);

  // Filter and sort items
  const filteredItems = useMemo(() => {
    let result = [...COST_ITEMS];

    if (selectedModule !== "All") {
      result = result.filter((item) => item.systemModule === selectedModule);
    }

    if (searchQuery.trim()) {
      const q = searchQuery.toLowerCase().trim();
      result = result.filter(
        (item) =>
          item.systemModule.toLowerCase().includes(q) ||
          item.itemDescription.toLowerCase().includes(q) ||
          (item.specification && item.specification.toLowerCase().includes(q))
      );
    }

    if (sortBy === "cost-desc") {
      result.sort((a, b) => b.subtotal - a.subtotal);
    } else if (sortBy === "cost-asc") {
      result.sort((a, b) => a.subtotal - b.subtotal);
    } else if (sortBy === "name") {
      result.sort((a, b) => a.itemDescription.localeCompare(b.itemDescription));
    }

    return result;
  }, [searchQuery, selectedModule, sortBy]);

  const currentFilteredTotal = useMemo(() => {
    return filteredItems.reduce((acc, item) => acc + item.subtotal, 0);
  }, [filteredItems]);

  const handleCopyCSV = () => {
    navigator.clipboard.writeText(RAW_CSV);
    setCopiedCSV(true);
    setTimeout(() => setCopiedCSV(false), 2000);
  };

  const handleDownloadCSV = () => {
    const blob = new Blob([RAW_CSV], { type: "text/csv;charset=utf-8;" });
    const url = URL.createObjectURL(blob);
    const link = document.createElement("a");
    link.href = url;
    link.setAttribute("download", "orbit_v1_detailed_cost_and_pricing_model.csv");
    document.body.appendChild(link);
    link.click();
    document.body.removeChild(link);
    URL.revokeObjectURL(url);
  };

  return (
    <div className="min-h-screen bg-white text-gray-900">
      {/* Top Header Bar */}
      <header className="border-b border-gray-200 bg-white">
        <div className="max-w-6xl mx-auto px-4 sm:px-6 py-4 flex flex-col sm:flex-row sm:items-center justify-between gap-3">
          <div className="flex items-center gap-3">
            <Link
              href="/"
              className="text-sm font-semibold text-gray-900 hover:text-gray-600"
            >
              SIH 2026
            </Link>
            <span className="text-gray-300">/</span>
            <span className="text-sm font-medium text-gray-900">Cost List</span>
          </div>

          <div className="flex items-center gap-2">
            <button
              id="btn-copy-csv"
              onClick={handleCopyCSV}
              className="px-3 py-1.5 text-xs font-medium border border-gray-300 rounded bg-white text-gray-700 hover:bg-gray-50"
            >
              {copiedCSV ? "Copied CSV" : "Copy CSV"}
            </button>
            <button
              id="btn-download-csv"
              onClick={handleDownloadCSV}
              className="px-3 py-1.5 text-xs font-medium border border-gray-300 rounded bg-white text-gray-700 hover:bg-gray-50"
            >
              Download CSV
            </button>
            <button
              id="btn-toggle-raw"
              onClick={() => setShowRawCsv(!showRawCsv)}
              className="px-3 py-1.5 text-xs font-medium border border-gray-300 rounded bg-gray-50 text-gray-700 hover:bg-gray-100"
            >
              {showRawCsv ? "Hide Raw CSV" : "Show Raw CSV"}
            </button>
          </div>
        </div>
      </header>

      {/* Main Content Container */}
      <main className="max-w-6xl mx-auto px-4 sm:px-6 py-8">
        {/* Page Title & Metadata */}
        <div className="mb-6 pb-6 border-b border-gray-200">
          <div className="text-xs font-medium text-gray-500 uppercase tracking-wider mb-1">
            Orbit V1 AGV + Telescopic Mast Subsystem
          </div>
          <h1 className="text-2xl sm:text-3xl font-bold text-gray-900 mb-2">
            Detailed Cost Breakdown
          </h1>
          <p className="text-sm text-gray-600 max-w-3xl">
            Complete financial breakdown covering component Bill of Materials (BOM), custom LFAM composite 3D printed chassis, CNC aluminum mast fabrication, assembly labor, and ROS2 software calibration.
          </p>
        </div>

        {/* Flat Summary KPI Grid */}
        <div className="grid grid-cols-2 md:grid-cols-4 gap-4 mb-6">
          <div className="border border-gray-200 rounded p-4 bg-gray-50">
            <div className="text-xs font-medium text-gray-500 uppercase">Factory Production Cost</div>
            <div className="text-2xl font-bold text-gray-900 mt-1">{TOTAL_FACTORY_COST_FORMATTED}</div>
            <div className="text-xs text-gray-500 mt-0.5">19 Total Line Items</div>
          </div>

          <div className="border border-gray-200 rounded p-4 bg-gray-50">
            <div className="text-xs font-medium text-gray-500 uppercase">AGV Direct Cost</div>
            <div className="text-2xl font-bold text-gray-900 mt-1">{PRICING_MODEL.agvDirectCostFormatted}</div>
            <div className="text-xs text-gray-500 mt-0.5">Base (₹77,050) + Chassis (₹37,500)</div>
          </div>

          <div className="border border-gray-200 rounded p-4 bg-gray-50">
            <div className="text-xs font-medium text-gray-500 uppercase">Mast Direct Cost</div>
            <div className="text-2xl font-bold text-gray-900 mt-1">{PRICING_MODEL.mastDirectCostFormatted}</div>
            <div className="text-xs text-gray-500 mt-0.5">Actuators (₹98,200) + CNC (₹37,500)</div>
          </div>

          <div className="border border-gray-200 rounded p-4 bg-gray-50">
            <div className="text-xs font-medium text-gray-500 uppercase">Assembly &amp; Testing</div>
            <div className="text-2xl font-bold text-gray-900 mt-1">{PRICING_MODEL.assemblyTestingCostFormatted}</div>
            <div className="text-xs text-gray-500 mt-0.5">Labor (₹18,000) + ROS2 (₹15,000)</div>
          </div>
        </div>



        {/* Raw CSV Box (collapsible) */}
        {showRawCsv && (
          <div className="mb-6 border border-gray-300 rounded bg-gray-50 p-4">
            <div className="flex items-center justify-between mb-2">
              <span className="text-xs font-semibold text-gray-700 uppercase">Raw CSV Data</span>
              <button
                onClick={handleCopyCSV}
                className="text-xs text-gray-600 hover:text-gray-900 underline"
              >
                {copiedCSV ? "Copied" : "Copy to clipboard"}
              </button>
            </div>
            <pre className="text-xs font-mono bg-white p-3 border border-gray-200 rounded overflow-x-auto text-gray-800 leading-relaxed">
              {RAW_CSV}
            </pre>
          </div>
        )}

        {/* Filter & Search Bar */}
        <div className="mb-4 flex flex-col sm:flex-row items-stretch sm:items-center justify-between gap-3">
          {/* Module Filter Buttons */}
          <div className="flex flex-wrap items-center gap-1">
            <button
              onClick={() => setSelectedModule("All")}
              className={`px-3 py-1.5 text-xs font-medium rounded border ${
                selectedModule === "All"
                  ? "bg-gray-900 text-white border-gray-900"
                  : "bg-white text-gray-700 border-gray-300 hover:bg-gray-50"
              }`}
            >
              All ({COST_ITEMS.length})
            </button>
            {MODULE_SUMMARIES.map((mod) => (
              <button
                key={mod.module}
                onClick={() => setSelectedModule(mod.module)}
                className={`px-3 py-1.5 text-xs font-medium rounded border ${
                  selectedModule === mod.module
                    ? "bg-gray-900 text-white border-gray-900"
                    : "bg-white text-gray-700 border-gray-300 hover:bg-gray-50"
                }`}
              >
                {mod.module} ({mod.count})
              </button>
            ))}
          </div>

          {/* Search Input & Sort */}
          <div className="flex items-center gap-2">
            <input
              id="search-input"
              type="text"
              placeholder="Search component or module..."
              value={searchQuery}
              onChange={(e) => setSearchQuery(e.target.value)}
              className="px-3 py-1.5 text-xs border border-gray-300 rounded bg-white text-gray-900 placeholder-gray-400 focus:outline-none focus:border-gray-500 w-full sm:w-56"
            />
            <select
              id="sort-select"
              value={sortBy}
              onChange={(e) => setSortBy(e.target.value as any)}
              className="px-3 py-1.5 text-xs border border-gray-300 rounded bg-white text-gray-700 focus:outline-none focus:border-gray-500 cursor-pointer"
            >
              <option value="default">Default Order</option>
              <option value="cost-desc">Cost: High to Low</option>
              <option value="cost-asc">Cost: Low to High</option>
              <option value="name">Item Name (A-Z)</option>
            </select>
          </div>
        </div>

        {/* Flat Structured Cost Table */}
        <div className="border border-gray-300 rounded overflow-hidden mb-8">
          <div className="overflow-x-auto">
            <table id="cost-list-table" className="w-full text-left border-collapse">
              <thead>
                <tr className="bg-gray-50 border-b border-gray-300 text-xs font-semibold text-gray-700">
                  <th scope="col" className="py-3 px-4">System Module</th>
                  <th scope="col" className="py-3 px-4">Item Description</th>
                  <th scope="col" className="py-3 px-4 text-center">Qty</th>
                  <th scope="col" className="py-3 px-4 text-right">Unit Cost (INR)</th>
                  <th scope="col" className="py-3 px-4 text-right">Subtotal (INR)</th>
                </tr>
              </thead>
              <tbody className="divide-y divide-gray-200 text-xs text-gray-800">
                {filteredItems.length === 0 ? (
                  <tr>
                    <td colSpan={5} className="py-8 text-center text-gray-500">
                      No matching components found for &quot;{searchQuery}&quot;
                    </td>
                  </tr>
                ) : (
                  filteredItems.map((item) => (
                    <tr key={item.id} className="hover:bg-gray-50/80">
                      <td className="py-3 px-4 font-medium text-gray-900 align-top whitespace-nowrap">
                        <div>{item.systemModule}</div>
                        <span className="inline-block mt-0.5 text-[11px] text-gray-500">
                          {item.category}
                        </span>
                      </td>
                      <td className="py-3 px-4 text-gray-700 align-top">
                        <div className="font-medium text-gray-900">{item.itemDescription}</div>
                        {item.specification && (
                          <div className="text-[11px] text-gray-500 mt-0.5">{item.specification}</div>
                        )}
                      </td>
                      <td className="py-3 px-4 text-center text-gray-900 font-mono align-top">
                        {item.qty}
                      </td>
                      <td className="py-3 px-4 text-right font-mono text-gray-700 align-top">
                        {item.unitCostFormatted}
                      </td>
                      <td className="py-3 px-4 text-right font-mono font-semibold text-gray-900 align-top">
                        {item.subtotalFormatted}
                      </td>
                    </tr>
                  ))
                )}
              </tbody>
              <tfoot>
                <tr className="border-t-2 border-gray-900 bg-gray-50 text-xs font-bold text-gray-900">
                  <td colSpan={2} className="py-3.5 px-4">
                    {selectedModule === "All" && !searchQuery
                      ? "TOTAL FACTORY PRODUCTION COST"
                      : `SUBTOTAL (${filteredItems.length} ITEMS)`}
                  </td>
                  <td className="py-3.5 px-4 text-center font-mono">
                    {filteredItems.reduce((acc, it) => acc + it.qty, 0)}
                  </td>
                  <td className="py-3.5 px-4 text-right text-gray-500 font-normal">
                    Total
                  </td>
                  <td className="py-3.5 px-4 text-right font-mono text-sm">
                    {selectedModule === "All" && !searchQuery
                      ? TOTAL_FACTORY_COST_FORMATTED
                      : `₹${currentFilteredTotal.toLocaleString("en-IN")}`}
                  </td>
                </tr>
              </tfoot>
            </table>
          </div>
        </div>


      </main>

      {/* Footer */}
      <footer className="border-t border-gray-200 py-6 text-center text-xs text-gray-500">
        Smart India Hackathon 2026 • Orbit V1 AGV &amp; Telescopic Mast System Documentation
      </footer>
    </div>
  );
}
