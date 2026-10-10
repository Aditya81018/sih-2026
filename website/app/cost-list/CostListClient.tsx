"use client";

import React, { useState, useMemo } from "react";
import Link from "next/link";
import {
  COST_ITEMS,
  TOTAL_COST,
  TOTAL_COST_FORMATTED,
  CATEGORY_SUMMARIES,
  RAW_CSV,
  CostItem,
} from "./data";

export default function CostListClient() {
  const [searchQuery, setSearchQuery] = useState("");
  const [selectedCategory, setSelectedCategory] = useState<string>("All");
  const [sortBy, setSortBy] = useState<"default" | "cost-desc" | "cost-asc" | "name">("default");
  const [copiedCSV, setCopiedCSV] = useState(false);
  const [showRawCsv, setShowRawCsv] = useState(false);

  // Filter and sort items
  const filteredItems = useMemo(() => {
    let result = [...COST_ITEMS];

    if (selectedCategory !== "All") {
      result = result.filter((item) => item.category === selectedCategory);
    }

    if (searchQuery.trim()) {
      const q = searchQuery.toLowerCase().trim();
      result = result.filter(
        (item) =>
          item.subsystemAxis.toLowerCase().includes(q) ||
          item.componentModel.toLowerCase().includes(q) ||
          item.category.toLowerCase().includes(q)
      );
    }

    if (sortBy === "cost-desc") {
      result.sort((a, b) => b.totalCost - a.totalCost);
    } else if (sortBy === "cost-asc") {
      result.sort((a, b) => a.totalCost - b.totalCost);
    } else if (sortBy === "name") {
      result.sort((a, b) => a.subsystemAxis.localeCompare(b.subsystemAxis));
    }

    return result;
  }, [searchQuery, selectedCategory, sortBy]);

  const currentFilteredTotal = useMemo(() => {
    return filteredItems.reduce((acc, item) => acc + item.totalCost, 0);
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
    link.setAttribute("download", "mast_subsystem_cost_list.csv");
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
            Endpoint: /cost-list
          </div>
          <h1 className="text-2xl sm:text-3xl font-bold text-gray-900 mb-2">
            Mast Subsystem Detailed Cost List
          </h1>
          <p className="text-sm text-gray-600 max-w-3xl">
            Detailed Bill of Materials (BOM) specification and component cost estimation for the primary telescopic mast, horizontal expansion arm, BLDC servo joints, stepper motors, controller, and vision sensor.
          </p>
        </div>

        {/* Flat Summary KPI Grid */}
        <div className="grid grid-cols-2 md:grid-cols-4 gap-4 mb-6">
          <div className="border border-gray-200 rounded p-4 bg-gray-50">
            <div className="text-xs font-medium text-gray-500 uppercase">Subsystem Total</div>
            <div className="text-2xl font-bold text-gray-900 mt-1">{TOTAL_COST_FORMATTED}</div>
            <div className="text-xs text-gray-500 mt-0.5">11 Line Items</div>
          </div>

          <div className="border border-gray-200 rounded p-4 bg-gray-50">
            <div className="text-xs font-medium text-gray-500 uppercase">BLDC Servos</div>
            <div className="text-2xl font-bold text-gray-900 mt-1">₹55,000</div>
            <div className="text-xs text-gray-500 mt-0.5">38.3% of total</div>
          </div>

          <div className="border border-gray-200 rounded p-4 bg-gray-50">
            <div className="text-xs font-medium text-gray-500 uppercase">Structure & Slides</div>
            <div className="text-2xl font-bold text-gray-900 mt-1">₹45,500</div>
            <div className="text-xs text-gray-500 mt-0.5">31.7% of total</div>
          </div>

          <div className="border border-gray-200 rounded p-4 bg-gray-50">
            <div className="text-xs font-medium text-gray-500 uppercase">Actuation & Vision</div>
            <div className="text-2xl font-bold text-gray-900 mt-1">₹43,200</div>
            <div className="text-xs text-gray-500 mt-0.5">30.0% of total</div>
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
          {/* Category Filter Buttons */}
          <div className="flex flex-wrap items-center gap-1">
            <button
              onClick={() => setSelectedCategory("All")}
              className={`px-3 py-1.5 text-xs font-medium rounded border ${
                selectedCategory === "All"
                  ? "bg-gray-900 text-white border-gray-900"
                  : "bg-white text-gray-700 border-gray-300 hover:bg-gray-50"
              }`}
            >
              All ({COST_ITEMS.length})
            </button>
            {CATEGORY_SUMMARIES.map((cat) => (
              <button
                key={cat.category}
                onClick={() => setSelectedCategory(cat.category)}
                className={`px-3 py-1.5 text-xs font-medium rounded border ${
                  selectedCategory === cat.category
                    ? "bg-gray-900 text-white border-gray-900"
                    : "bg-white text-gray-700 border-gray-300 hover:bg-gray-50"
                }`}
              >
                {cat.category} ({cat.count})
              </button>
            ))}
          </div>

          {/* Search Input & Sort */}
          <div className="flex items-center gap-2">
            <input
              id="search-input"
              type="text"
              placeholder="Search component or axis..."
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
              <option value="name">Axis Name (A-Z)</option>
            </select>
          </div>
        </div>

        {/* Clean, Plain, Flat Table */}
        <div className="border border-gray-300 rounded overflow-hidden mb-8">
          <div className="overflow-x-auto">
            <table id="cost-list-table" className="w-full text-left border-collapse">
              <thead>
                <tr className="bg-gray-50 border-b border-gray-300 text-xs font-semibold text-gray-700">
                  <th scope="col" className="py-3 px-4">Subsystem / Axis</th>
                  <th scope="col" className="py-3 px-4">Component Model / Specification</th>
                  <th scope="col" className="py-3 px-4 text-center">Qty</th>
                  <th scope="col" className="py-3 px-4 text-right">Est. Unit Cost (INR)</th>
                  <th scope="col" className="py-3 px-4 text-right">Total Cost (INR)</th>
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
                      <td className="py-3 px-4 font-medium text-gray-900 align-top">
                        <div>{item.subsystemAxis}</div>
                        <span className="inline-block mt-0.5 text-[11px] text-gray-500">
                          {item.category}
                        </span>
                      </td>
                      <td className="py-3 px-4 text-gray-700 align-top">
                        {item.componentModel}
                      </td>
                      <td className="py-3 px-4 text-center text-gray-900 font-mono align-top">
                        {item.qty}
                      </td>
                      <td className="py-3 px-4 text-right font-mono text-gray-700 align-top">
                        {item.unitCostFormatted}
                      </td>
                      <td className="py-3 px-4 text-right font-mono font-semibold text-gray-900 align-top">
                        {item.totalCostFormatted}
                      </td>
                    </tr>
                  ))
                )}
              </tbody>
              <tfoot>
                <tr className="border-t-2 border-gray-900 bg-gray-50 text-xs font-bold text-gray-900">
                  <td colSpan={2} className="py-3.5 px-4">
                    {selectedCategory === "All" && !searchQuery
                      ? "MAST SUBSYSTEM TOTAL"
                      : `SUBTOTAL (${filteredItems.length} ITEMS)`}
                  </td>
                  <td className="py-3.5 px-4 text-center font-mono">
                    {filteredItems.reduce((acc, it) => acc + it.qty, 0)}
                  </td>
                  <td className="py-3.5 px-4 text-right text-gray-500 font-normal">
                    Total
                  </td>
                  <td className="py-3.5 px-4 text-right font-mono text-sm">
                    {selectedCategory === "All" && !searchQuery
                      ? TOTAL_COST_FORMATTED
                      : `₹${currentFilteredTotal.toLocaleString("en-IN")}`}
                  </td>
                </tr>
              </tfoot>
            </table>
          </div>
        </div>

        {/* API Reference Box */}
        <div className="border border-gray-200 rounded p-4 bg-gray-50 text-xs">
          <div className="font-semibold text-gray-800 mb-1">API Endpoints</div>
          <div className="text-gray-600 mb-2">
            The data is also accessible directly via GET requests:
          </div>
          <div className="grid grid-cols-1 sm:grid-cols-2 gap-2 font-mono text-[11px]">
            <div className="p-2 border border-gray-300 rounded bg-white text-gray-800">
              <span className="text-gray-500">JSON: </span>GET /api/cost-list
            </div>
            <div className="p-2 border border-gray-300 rounded bg-white text-gray-800">
              <span className="text-gray-500">CSV: </span>GET /api/cost-list?format=csv
            </div>
          </div>
        </div>
      </main>

      {/* Footer */}
      <footer className="border-t border-gray-200 py-6 text-center text-xs text-gray-500">
        Smart India Hackathon 2026 • Mast Subsystem Documentation
      </footer>
    </div>
  );
}
